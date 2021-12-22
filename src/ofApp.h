
//
//  Starter file for Project 3 - Skeleton Builder
//
//  This file includes functionality that supports selection and translate/rotation
//  of scene objects using the mouse.
//
//  Modifer keys for rotatation are x, y and z keys (for each axis of rotation)
//
//  (c) Kevin M. Smith  - 24 September 2018
//

#include "ofMain.h"
#include "ofxGui.h"
//#include "box.h" // This was causing errors, had to combine back into ofApp.h/.cpp
//#include "Primitives.h" // This was causing errors, had to combine back into ofApp.h/.cpp

#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/intersect.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/quaternion.hpp"

#include <assert.h>
#include "vector3.h"
#include "ray.h"

/*
 * Axis-aligned bounding box class, for use with the optimized ray-box
 * intersection test described in:
 *
 *      Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 *      "An Efficient and Robust Ray-Box Intersection Algorithm"
 *      Journal of graphics tools, 10(1):49-54, 2005
 *
 */

class Box {
public:
	Box() { }
	Box(const Vector3 &min, const Vector3 &max) {
		//     assert(min < max);
		parameters[0] = min;
		parameters[1] = max;
	}
	// (t0, t1) is the interval for valid hits
	bool intersect(const _Ray &, float t0, float t1) const;

	// corners
	Vector3 parameters[2];
	Vector3 min() { return parameters[0]; }
	Vector3 max() { return parameters[1]; }
	const bool inside(const Vector3 &p) {
		return ((p.x() >= parameters[0].x() && p.x() <= parameters[1].x()) &&
			(p.y() >= parameters[0].y() && p.y() <= parameters[1].y()) &&
			(p.z() >= parameters[0].z() && p.z() <= parameters[1].z()));
	}
	const bool inside(Vector3 *points, int size) {
		bool allInside = true;
		for (int i = 0; i < size; i++) {
			if (!inside(points[i])) allInside = false;
			break;
		}
		return allInside;
	}
	Vector3 center() {
		return ((max() - min()) / 2 + min());
	}
};


//  General Purpose Ray class 
//
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d;
};

//  Base class for any renderable object in the scene
//
class SceneObject {
public:
	virtual void draw() = 0;    // pure virtual funcs - must be overloaded
	virtual bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }
	virtual void update() { } // Do nothing unless overridden

	// commonly used transformations
	//
	glm::mat4 getRotateMatrix() {
		return (glm::eulerAngleYXZ(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z)));   // yaw, pitch, roll 
	}
	glm::mat4 getTranslateMatrix() {
		return (glm::translate(glm::mat4(1.0), glm::vec3(position.x, position.y, position.z)));
	}
	glm::mat4 getScaleMatrix() {
		return (glm::scale(glm::mat4(1.0), glm::vec3(scale.x, scale.y, scale.z)));
	}


	glm::mat4 getLocalMatrix() {

		// get the local transformations + pivot
		//
		glm::mat4 scale = getScaleMatrix();
		glm::mat4 rotate = getRotateMatrix();
		glm::mat4 trans = getTranslateMatrix();

		// handle pivot point  (rotate around a point that is not the object's center)
		//
		glm::mat4 pre = glm::translate(glm::mat4(1.0), glm::vec3(-pivot.x, -pivot.y, -pivot.z));
		glm::mat4 post = glm::translate(glm::mat4(1.0), glm::vec3(pivot.x, pivot.y, pivot.z));



		return (trans * post * rotate * pre * scale);

	}

	glm::mat4 getMatrix() {

		// if we have a parent (we are not the root),
		// concatenate parent's transform (this is recursive)
		// 
		if (parent) {
			glm::mat4 M = parent->getMatrix();
			return (M * getLocalMatrix());
		}
		else return getLocalMatrix();  // priority order is SRT
	}

	// get current Position in World Space
	//
	glm::vec3 getPosition() {
		return (getMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0));
	}

	// set position (pos is in world space)
	//
	void setPosition(glm::vec3 pos) {
		position = glm::inverse(getMatrix()) * glm::vec4(pos, 1.0);
	}

	// return a rotation  matrix that rotates one vector to another
	//
	glm::mat4 rotateToVector(glm::vec3 v1, glm::vec3 v2);

	//  Hierarchy 
	//
	void addChild(SceneObject *child) {
		childList.push_back(child);
		child->parent = this;
	}

	SceneObject *parent = NULL;        // if parent = NULL, then this obj is the ROOT
	vector<SceneObject *> childList;

	// position/orientation 
	//
	glm::vec3 position = glm::vec3(0, 0, 0);   // translate
	glm::vec3 rotation = glm::vec3(0, 0, 0);   // rotate
	glm::vec3 scale = glm::vec3(1, 1, 1);      // scale

	// rotate pivot
	//
	glm::vec3 pivot = glm::vec3(0, 0, 0);

	// material properties (we will ultimately replace this with a Material class - TBD)
	//
	ofColor diffuseColor = ofColor::grey;    // default colors - can be changed.
	ofColor specularColor = ofColor::lightGray;

	// UI parameters
	//
	bool isSelectable = true;
	string name = "SceneObject";
};

class Cone : public SceneObject {
public:
	Cone(ofColor color = ofColor::blue) {
		diffuseColor = color;
	}
	Cone(glm::vec3 tran, glm::vec3 rot, glm::vec3 sc, ofColor color = ofColor::blue) {
		position = tran;
		rotation = rot;
		scale = sc;
		diffuseColor = color;
	}
	void draw();
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal);

	float radius = 1.0;
	float height = 2.0;
};

class Cube : public SceneObject {
public:
	Cube(ofColor color = ofColor::blue) {
		diffuseColor = color;
	}
	Cube(glm::vec3 tran, glm::vec3 rot, glm::vec3 sc, ofColor color = ofColor::blue) {
		position = tran;
		rotation = rot;
		scale = sc;
		diffuseColor = color;
	}
	void draw();
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal);

	float width = 2.0;
	float height = 2.0;
	float depth = 2.0;
};

//  General purpose sphere  (assume parametric)
//
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::lightGray) { position = p; radius = r; diffuseColor = diffuse; }
	Sphere() {}
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal);
	void draw();

	float radius = 1.0;
};


//  Mesh class (will complete later- this will be a refinement of Mesh from Project 1)
//
class Mesh : public SceneObject {
	bool intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) { return false; }
	void draw() { }
};


//  General purpose plane 
//
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::darkGreen, float w = 20, float h = 20) {
		position = p; normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		isSelectable = false;
		plane.rotateDeg(-90, 1, 0, 0);
		plane.setPosition(position);
		plane.setWidth(width);
		plane.setHeight(height);
		plane.setResolution(4, 4);

	}
	Plane() {
		plane.rotateDeg(-90, 1, 0, 0);
		isSelectable = false;
	}
	glm::vec3 normal = glm::vec3(0, 1, 0);
	bool intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normal);
	void draw() {
		material.begin();
		material.setDiffuseColor(diffuseColor);
		plane.drawFaces();
		material.end();
	}
	ofPlanePrimitive plane;
	ofMaterial material;

	float width = 20;
	float height = 20;
};


// Skeleton stuff

class Joint : public Sphere {
public:
	Joint(string n, glm::vec3 p, glm::vec3 rot, glm::vec3 trans, Joint* parent = NULL, ofColor diffuse = ofColor::lightGray) {
		glm::vec3 relTrans = p + trans;
		setPosition(relTrans);
		rotation = rot;
		radius = defaultRadius;
		diffuseColor = diffuse;
		if (parent != NULL) {
			parent->addChild(this);
		}
		isSelectable = true;
		name = n;
		axisIsLocked = false;
		startOffset = p;
	}

	float defaultRadius = 0.5;
	bool axisIsLocked;
	glm::vec3 lockedAxis;
	glm::vec3 startOffset;

	void draw();
};

// IK Stuff
// Uses gradient descent to animate an inverse kinematics arm
class IKArm : public SceneObject {
private:
	glm::vec3 normX = { 1, 0, 0 };
	glm::vec3 normY = { 0, 1, 0 };
	glm::vec3 normZ = { 0, 0, 1 };

public:
	IKArm(vector<Joint*> joints_, Joint* target_) {
		joints = joints_;
		for (int i = 0; i < joints.size(); i++) {
			auto joint = joints[i];
			if (i == 0) { // Base joint
				joint->lockedAxis = normY;
				//angles.push_back(joint->rotation.y);
			}
			else {
				joint->lockedAxis = normZ;
				//angles.push_back(joint->rotation.z);
			}
			joint->axisIsLocked = true;
		}
		target = target_;
		isSelectable = false;
	}
	void setAngles(vector<float> angles) {
		for (int i = 0; i < joints.size(); i++) {
			auto joint = joints[i];
			auto angle = angles[i];
			if (joint->lockedAxis == normX) {
				joint->rotation.x = angle;
			}
			else if (joint->lockedAxis == normY) {
				joint->rotation.y = angle;
			}
			else if (joint->lockedAxis == normZ) {
				joint->rotation.z = angle;
			}
		}
	}
	void applyAngles() {
		vector<float> angles = getAngles();
		for (int i = 0; i < joints.size(); i++) {
			auto joint = joints[i];
			auto angle = angles[i];
			if (joint->lockedAxis == normX) {
				joint->rotation.x = angle;
			}
			else if (joint->lockedAxis == normY) {
				joint->rotation.y = angle;
			}
			else if (joint->lockedAxis == normZ) {
				joint->rotation.z = angle;
			}
		}
	}
	vector<float> getAngles() {
		vector<float> angles;
		for (int i = 0; i < joints.size(); i++) {
			auto joint = joints[i];
			if (joint->lockedAxis == normX) {
				angles.push_back(joint->rotation.x);
			}
			else if (joint->lockedAxis == normY) {
				angles.push_back(joint->rotation.y);
			}
			else if (joint->lockedAxis == normZ) {
				angles.push_back(joint->rotation.z);
			}
		}
		return angles;
	}
	glm::vec3 simulateRotations(vector<float> jointAngles); 
	float distanceToTarget(vector<float> jointAngles); 
	float gradient(vector<float> jointAngles, int jointIndex); 
	void moveTowardsTarget(); 
	void update() {
		moveTowardsTarget();
	}
	void draw() {
		/*for (auto joint : joints) {
			ofSetColor(diffuseColor);
			joint->draw();
		}*/
	}

	vector<Joint*> joints;

	Joint* target;
	static ofParameter<float> learningRate; // Rate of change of the gradient after calculation
	static ofParameter<float> deltaRotation; // Size of each rotation jump during gradient descent
	static ofParameter<float> distThreshold; // Maximum acceptable distance - if within, don't move closer
};


// Keyframing stuff
typedef struct {
	glm::vec3 position;
	glm::vec3 rotation;
} SceneObjectInfo;

class KeyFrame {
public:
	KeyFrame(vector<SceneObject*> scene_) {
		for (auto obj : scene_) {
			SceneObjectInfo info;
			info.position = obj->position;
			info.rotation = obj->rotation;
			scene.push_back(info);
		}
	}

	vector<SceneObjectInfo> scene;
};

class Animation {
public:
	Animation(vector<SceneObject*>* liveScene_) {
		liveScene = liveScene_;
		paused = true;
		hasReachedKeyFrame = false;
	}
	void update();
	void animate();
	void start();
	void reset();
	void togglePause();

	float getTimePerKeyFrame() { 
		if(keyFrames.size() > 0) return lengthInSeconds / keyFrames.size(); 
		else return -1;
	}

	float getTimeSinceLastKeyFrame() {
		return ofGetElapsedTimef() - timeAtLastKeyFrame;
	}

	void addFrameFromScene();
	void applyStartKeyFrame();

	vector<KeyFrame> keyFrames;
	int currentFrameIdx;
	int nextFrameIdx;
	bool paused;
	bool hasReachedKeyFrame;
	float timeAtLastKeyFrame;

	vector<SceneObject*>* liveScene;
	static ofParameter<float> lengthInSeconds; // How long the animation takes 
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		static void drawAxis(glm::mat4 transform = glm::mat4(1.0), float len = 1.0);
		bool mouseToDragPlane(int x, int y, glm::vec3 &point);
		void printChannels(SceneObject *);
		bool objSelected() { return (selected.size() ? true : false ); };
		
		// Lights
		//
		ofLight light1;
	
		// Cameras
		//
		ofEasyCam  mainCam;
		ofCamera sideCam;
		ofCamera topCam;
		ofCamera  *theCam;    // set to current camera either mainCam or sideCam

		// Materials
		//
		ofMaterial material;


		// scene components
		//
		vector<SceneObject *> scene;
		vector<SceneObject *> selected;
		ofPlanePrimitive plane;

		// state
		bool bDrag = false;
		bool bHide = true;
		bool bAltKeyDown = false;
		bool bRotateX = false;
		bool bRotateY = false;
		bool bRotateZ = false;
		glm::vec3 lastPoint;
		void clearScene();

		// Skeleton
		void spawnJoint();
		void spawnJoint(string name, glm::vec3 rot, glm::vec3 trans, Joint* parent);
		int numJointsSpawned = 0;
		void deleteSelected();
		void saveToFile();
		void loadFromFile(string filename);
		SceneObject* findObjFromName(string name);

		// IK
		void startIK();


		// Animation
		Animation* animation = nullptr;
		float animationLength = 3; // Seconds
		void handleKeyFrameSave();
		void handleStartAnimation();
		void handleToggleAnimationPause();

		// GUI
		ofxPanel gui;
};
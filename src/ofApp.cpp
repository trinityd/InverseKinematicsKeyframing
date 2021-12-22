
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


#include "ofApp.h"

#include "vector3.h"
#include "ray.h"
#include <fstream> 

//#include "box.h"

/*
 * Ray-box intersection using IEEE numerical properties to ensure that the
 * test is both robust and efficient, as described in:
 *
 *      Amy Williams, Steve Barrus, R. Keith Morley, and Peter Shirley
 *      "An Efficient and Robust Ray-Box Intersection Algorithm"
 *      Journal of graphics tools, 10(1):49-54, 2005
 *
 */


bool Box::intersect(const _Ray &r, float t0, float t1) const {
	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	tmin = (parameters[r.sign[0]].x() - r.origin.x()) * r.inv_direction.x();
	tmax = (parameters[1 - r.sign[0]].x() - r.origin.x()) * r.inv_direction.x();
	tymin = (parameters[r.sign[1]].y() - r.origin.y()) * r.inv_direction.y();
	tymax = (parameters[1 - r.sign[1]].y() - r.origin.y()) * r.inv_direction.y();
	if ((tmin > tymax) || (tymin > tmax))
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;
	tzmin = (parameters[r.sign[2]].z() - r.origin.z()) * r.inv_direction.z();
	tzmax = (parameters[1 - r.sign[2]].z() - r.origin.z()) * r.inv_direction.z();
	if ((tmin > tzmax) || (tzmin > tmax))
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;
	return ((tmin < t1) && (tmax > t0));
}

// Primitives stuff, had to bring it into this file cuz as a separate one it was causing errors

// Generate a rotation matrix that rotates v1 to v2
// v1, v2 must be normalized
//
glm::mat4 SceneObject::rotateToVector(glm::vec3 v1, glm::vec3 v2) {

	glm::vec3 axis = glm::cross(v1, v2);
	glm::quat q = glm::angleAxis(glm::angle(v1, v2), glm::normalize(axis));
	return glm::toMat4(q);
}


// Draw a Unit cube (size = 2) transformed 
//
void Cone::draw() {

	//rotateToVector(glm::vec3(0, 1, 0), glm::vec3(1, 1, 1));

	glm::mat4 m = getMatrix();

	//   push the current stack matrix and multiply by this object's
	//   matrix. now all vertices will be transformed by this matrix
	//
	ofPushMatrix();
	ofMultMatrix(m);
	ofDrawCone(radius, height);
	ofPopMatrix();


	// draw axis
	//
	ofApp::drawAxis(m, 1.5);

}

//  Cone::intersect - test intersection with bounding box.  Note that
//  intersection test is done in object space with an axis aligned box (AAB), 
//  the input ray is provided in world space, so we need to transform the ray to object space.
//  this method does NOT return a normal.
//
bool Cone::intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {

	// transform Ray to object space.  
	//
	glm::mat4 mInv = glm::inverse(getMatrix());
	glm::vec4 p = mInv * glm::vec4(ray.p.x, ray.p.y, ray.p.z, 1.0);
	glm::vec4 p1 = mInv * glm::vec4(ray.p + ray.d, 1.0);
	glm::vec3 d = glm::normalize(p1 - p);


	// intesect method we use will be Willam's  (see box.h and box.cc for reference).
	// note that this class has it's own version of Ray, Vector3  (TBD: port to GLM)
	//
	_Ray boxRay = _Ray(Vector3(p.x, p.y, p.z), Vector3(d.x, d.y, d.z));

	// we will test for intersection in object space (object is a "unit" cube edge is len=2)
	//
	Box box = Box(Vector3(-radius, -radius, 0), Vector3(radius, radius, height));
	return (box.intersect(boxRay, -1000, 1000));


}



// Draw a Unit cube (size = 2) transformed 
//
void Cube::draw() {

	//   get the current transformation matrix for this object
	//
	glm::mat4 m = getMatrix();

	//   push the current stack matrix and multiply by this object's
	//   matrix. now all vertices dran will be transformed by this matrix
	//
	ofPushMatrix();
	ofMultMatrix(m);
	ofDrawBox(width, height, depth);
	ofPopMatrix();

	// draw axis
	//
	ofApp::drawAxis(m, 1.5);

}

void Sphere::draw() {

	//   get the current transformation matrix for this object
   //
	glm::mat4 m = getMatrix();

	//   push the current stack matrix and multiply by this object's
	//   matrix. now all vertices dran will be transformed by this matrix
	//
	ofPushMatrix();
	ofMultMatrix(m);
	ofDrawSphere(radius);
	ofPopMatrix();

	// draw axis
	//
	ofApp::drawAxis(m, 1.5);

}

bool Sphere::intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {

	// transform Ray to object space.  
	//
	glm::mat4 mInv = glm::inverse(getMatrix());
	glm::vec4 p = mInv * glm::vec4(ray.p.x, ray.p.y, ray.p.z, 1.0);
	glm::vec4 p1 = mInv * glm::vec4(ray.p + ray.d, 1.0);
	glm::vec3 d = glm::normalize(p1 - p);

	return (glm::intersectRaySphere(glm::vec3(p), d, glm::vec3(0, 0, 0), radius, point, normal));
}




//  Cube::intersect - test intersection with the unit Cube.  Note that
//  intersection test is done in object space with an axis aligned box (AAB), 
//  the input ray is provided in world space, so we need to transform the ray to object space.
//  this method does NOT return a normal.
//
bool Cube::intersect(const Ray &ray, glm::vec3 &point, glm::vec3 &normal) {

	// transform Ray to object space.  
	//
	glm::mat4 mInv = glm::inverse(getMatrix());
	glm::vec4 p = mInv * glm::vec4(ray.p.x, ray.p.y, ray.p.z, 1.0);
	glm::vec4 p1 = mInv * glm::vec4(ray.p + ray.d, 1.0);
	glm::vec3 d = glm::normalize(p1 - p);


	// intesect method we use will be Willam's  (see box.h and box.cc for reference).
	// note that this class has it's own version of Ray, Vector3  (TBD: port to GLM)
	//
	_Ray boxRay = _Ray(Vector3(p.x, p.y, p.z), Vector3(d.x, d.y, d.z));

	// we will test for intersection in object space (object is a "unit" cube edge is len=2)
	//
	Box box = Box(Vector3(-width / 2.0, -height / 2.0, -depth / 2.0), Vector3(width / 2.0, height / 2.0, depth / 2.0));
	return (box.intersect(boxRay, -1000, 1000));

}


// Intersect Ray with Plane  (wrapper on glm::intersect*)
//
bool Plane::intersect(const Ray &ray, glm::vec3 & point, glm::vec3 & normalAtIntersect) {
	float dist;
	bool insidePlane = false;
	bool hit = glm::intersectRayPlane(ray.p, ray.d, position, this->normal, dist);
	if (hit) {
		Ray r = ray;
		point = r.evalPoint(dist);
		normalAtIntersect = this->normal;
		glm::vec2 xrange = glm::vec2(position.x - width / 2, position.x + width / 2);
		glm::vec2 zrange = glm::vec2(position.z - height / 2, position.z + height / 2);
		if (point.x < xrange[1] && point.x > xrange[0] && point.z < zrange[1] && point.z > zrange[0]) {
			insidePlane = true;
		}
	}
	return insidePlane;
}

//--------------------------------------------------------------
//
void ofApp::setup() {
	// GUI
	gui.setup();
	gui.add(IKArm::learningRate);
	gui.add(IKArm::deltaRotation);
	gui.add(IKArm::distThreshold);
	gui.add(Animation::lengthInSeconds);

	ofSetBackgroundColor(ofColor::black);
	mainCam.setDistance(15);
	mainCam.setNearClip(.1);
	
	sideCam.setPosition(40, 0, 0);
	sideCam.lookAt(glm::vec3(0, 0, 0));
	topCam.setNearClip(.1);
	topCam.setPosition(0, 16, 0);
	topCam.lookAt(glm::vec3(0, 0, 0));
	ofSetSmoothLighting(true);


	// setup one point light
	//
	light1.enable();
	light1.setPosition(5, 5, 0);
	light1.setDiffuseColor(ofColor(255.f, 255.f, 255.f));
	light1.setSpecularColor(ofColor(255.f, 255.f, 255.f));

	theCam = &mainCam;

	//  create a scene consisting of a ground plane with 2x2 blocks
	//  arranged in semi-random positions, scales and rotations
	//
	// ground plane
	//
	scene.push_back(new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0)));   

}

 
//--------------------------------------------------------------
void ofApp::update() {
	for (auto obj : scene) obj->update();
	if(animation != nullptr) animation->update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofEnableDepthTest();

	theCam->begin();
	ofNoFill();
	drawAxis();
	ofEnableLighting();

	//  draw the objects in scene
	//
	material.begin();
	ofFill();
	for (int i = 0; i < scene.size(); i++) {
		if (objSelected() && scene[i] == selected[0])
			ofSetColor(ofColor::purple);
		else ofSetColor(scene[i]->diffuseColor);
		scene[i]->draw();
	}

	material.end();
	ofDisableLighting();
	theCam->end();

	ofDisableDepthTest();
	gui.draw();
}

// 
// Draw an XYZ axis in RGB at transform
//
void ofApp::drawAxis(glm::mat4 m, float len) {

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(len, 0, 0, 1)));


	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, len, 0, 1)));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, 0, len, 1)));
}

// print C++ code for obj tranformation channels. (for debugging);
//
void ofApp::printChannels(SceneObject *obj) {
	cout << "position = glm::vec3(" << obj->position.x << "," << obj->position.y << "," << obj->position.z << ");" << endl;
	cout << "rotation = glm::vec3(" << obj->rotation.x << "," << obj->rotation.y << "," << obj->rotation.z << ");" << endl;
	cout << "scale = glm::vec3(" << obj->scale.x << "," << obj->scale.y << "," << obj->scale.z << ");" << endl;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

	switch (key) {
	case OF_KEY_ALT:
		bAltKeyDown = false;
		mainCam.disableMouseInput();
		break;
	case 'x':
		bRotateX = false;
		break;
	case 'y':
		bRotateY = false;
		break;
	case 'z':
		bRotateZ = false;
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case 's':
		startIK();
		break;
	case 'd':
	{
		clearScene();
		if (animation != nullptr) animation->reset();
	}
		break;
	case 'q':
		handleKeyFrameSave();
		break;
	case 'w':
		handleStartAnimation();
		break;
	case 'e':
		handleToggleAnimationPause();
		break;
	case 'C':
	case 'c':
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case 'F':
	case 'b':
		break;
	case 'f':
		ofToggleFullscreen();
		break;
	case 'h':
		bHide = !bHide;
		break;
	case 'i':
		break;
	case 'j':
		spawnJoint();
		break;
	case 'k':
		deleteSelected();
		break;
	case 'l':
		saveToFile();
		break;
	case 'p':
		if (objSelected()) printChannels(selected[0]);
		break;
	case 'r':
		break;
	case 'x':
		bRotateX = true;
		break;
	case 'y':
		bRotateY = true;
		break;
	case 'z':
		bRotateZ = true;
		break;
	case OF_KEY_F1: 
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3:
		theCam = &topCam;
		break;
	case OF_KEY_ALT:
		bAltKeyDown = true;
		if (!mainCam.getMouseInputEnabled()) mainCam.enableMouseInput();
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	if (objSelected() && bDrag) {
		glm::vec3 point; 
		mouseToDragPlane(x, y, point);
		if (bRotateX) {
			selected[0]->rotation += glm::vec3((point.x - lastPoint.x) * 20.0, 0, 0);
		}
		else if (bRotateY) {
			selected[0]->rotation += glm::vec3(0, (point.x - lastPoint.x) * 20.0, 0);
		}
		else if (bRotateZ) {
			selected[0]->rotation += glm::vec3(0, 0, (point.x - lastPoint.x) * 20.0);
		}
		else {
			selected[0]->position += (point - lastPoint);
		}
		lastPoint = point;
	}

}

//  This projects the mouse point in screen space (x, y) to a 3D point on a plane
//  normal to the view axis of the camera passing through the point of the selected object.
//  If no object selected, the plane passing through the world origin is used.
//
bool ofApp::mouseToDragPlane(int x, int y, glm::vec3 &point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	float dist;
	glm::vec3 pos;
	if (objSelected()) {
		pos = selected[0]->position;
	}
	else pos = glm::vec3(0, 0, 0);
	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}
	return false;
}

//--------------------------------------------------------------
//
// Provides functionality of single selection and if something is already selected,
// sets up state for translation/rotation of object using mouse.
//
void ofApp::mousePressed(int x, int y, int button){

	// if we are moving the camera around, don't allow selection
	//
	if (mainCam.getMouseInputEnabled()) return;

	// clear selection list
	//
	selected.clear();

	//
	// test if something selected
	//
	vector<SceneObject *> hits;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of scene objects
	//
	for (int i = 0; i < scene.size(); i++) {
		
		glm::vec3 point, norm;
		
		//  We hit an object
		//
		if (scene[i]->isSelectable && scene[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(scene[i]);
		}
	}


	// if we selected more than one, pick nearest
	//
	SceneObject *selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = std::numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(hits[n]->position - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}	
		}
	}
	if (selectedObj) {
		selected.push_back(selectedObj);
		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
	}
	else {
		selected.clear();
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	bDrag = false;

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
	if (dragInfo.files.size() == 1) {
		string fileName = dragInfo.files[0];
		loadFromFile(fileName);
	}
}


// Skeleton stuff
void ofApp::spawnJoint() {
	Joint* parent = NULL;
	if (objSelected()) {
		SceneObject* firstSelected = selected[0];
		if (dynamic_cast<Joint*>(firstSelected) != nullptr) { // A joint is selected
			parent = dynamic_cast<Joint*>(firstSelected);
		}
	}
	string name = "joint" + to_string(numJointsSpawned);
	glm::vec3 rot = { 0, 0, 0 };
	glm::vec3 trans = { 0, 0, 0 };
	spawnJoint(name, rot, trans, parent);
}

SceneObject* ofApp::findObjFromName(string name) {
	for (auto obj : scene) {
		if (obj->name == name) return obj;
	}
}

void ofApp::spawnJoint(string name, glm::vec3 rot, glm::vec3 trans, Joint* parent = NULL) {
	glm::vec3 pos = { 0, 0, 0 }; // Spawn on top of either origin or parent
	Joint* joint = new Joint(name, pos, rot, trans, parent);
	scene.push_back(joint);
	numJointsSpawned++;
}

void ofApp::deleteSelected() {
	if (objSelected()) {
		SceneObject* selectedObj = selected[0];
		SceneObject* newParentForChildren = selectedObj->parent;
		if (newParentForChildren != NULL) {
			for (auto child : selectedObj->childList) {
				newParentForChildren->addChild(child);
			}
		}
		else {
			for (auto child : selectedObj->childList) {
				child->parent = NULL;
			}
		}
		auto iteratorAtIndexOfObject = find(scene.begin(), scene.end(), selectedObj);
		scene.erase(iteratorAtIndexOfObject);
		selected.erase(selected.begin());
	}
}

void ofApp::saveToFile() {
	if (scene.size() > 1) { // Need more than plane
		string skeletonFileName = "skeleton.txt";
		string cwd = ofFilePath::getCurrentWorkingDirectory();

		cout << "Saving to file " << cwd << "\\" << skeletonFileName << "..." << endl;

		ofstream write;
		write.open(skeletonFileName);
		// Format:
		// create -joint “joint name” -rotate “<x, y, z>” -translate “<x, y, z>” -parent “name”; 
		for (int i = 1; i < scene.size(); i++) { // Skip plane
			SceneObject* curr = scene[i];
			if (dynamic_cast<Joint*>(curr) != nullptr) { // Only write Joints - ignore all others for now
				write << "create -joint " << curr->name
					<< " -rotate <" << curr->rotation
					<< "> -translate <" << curr->position << "> ";
				if (curr->parent != NULL) {
					write << "-parent " << curr->parent->name;
				}
				write << "\n";
			}
		}
		write.close();
		cout << "Done." << endl;
	}
	else {
		cout << "There's no skeleton to save." << endl;
	}
}

void replaceAll(std::string & orig, std::string findStr, std::string replaceStr) // Helper
{
	size_t pos = orig.find(findStr);
	while (pos != std::string::npos)
	{
		orig.replace(pos, findStr.size(), replaceStr);
		pos = orig.find(findStr, pos + replaceStr.size());
	}
}

void ofApp::clearScene() {
	scene.erase(scene.begin() + 1, scene.end());
	selected.clear();
}

void ofApp::loadFromFile(string filename) {
	ifstream in(filename);
	if (!in) {
		cout << "Invalid file: " << filename << endl;
	}
	else {
		cout << "Loading from file: " << filename << endl;

		clearScene();

		// Format:
		// create -joint “joint name” -rotate “<x, y, z>” -translate “<x, y, z>” -parent “name”; 
		string line;
		while (getline(in, line)) {
			string jointName;
			glm::vec3 jointRot = { 0, 0, 0 };
			glm::vec3 jointTrans = { 0, 0, 0 };
			string parentName = "null";
			Joint* jointParent = NULL;

			string s1, s2;
			float f1, f2, f3;
			istringstream iss(line);
			while (iss >> s1) {
				if (s1 == "-joint") {
					iss >> jointName;
				}
				else if (s1 == "-rotate") {
					iss >> s2;
					replaceAll(s2, ",", "");
					replaceAll(s2, "<", "");
					stringstream ss1(s2);
					ss1 >> f1;

					iss >> s2;
					stringstream ss2(s2);
					ss2 >> f2;
					replaceAll(s2, ",", "");

					iss >> s2;
					replaceAll(s2, ",", "");
					replaceAll(s2, ">", "");
					stringstream ss3(s2);
					ss3 >> f3;

					jointRot = { f1, f2, f3 };
				}
				else if (s1 == "-translate") {
					iss >> s2;
					replaceAll(s2, ",", "");
					replaceAll(s2, "<", "");
					stringstream ss1(s2);
					ss1 >> f1;

					iss >> s2;
					stringstream ss2(s2);
					ss2 >> f2;
					replaceAll(s2, ",", "");

					iss >> s2;
					replaceAll(s2, ",", "");
					replaceAll(s2, ">", "");
					stringstream ss3(s2);
					ss3 >> f3;

					jointTrans = { f1, f2, f3 };
				}
				else if (s1 == "-parent") {
					iss >> parentName;
				}
			} // Ignore everything else for now

			if (parentName != "null") {
				SceneObject* obj = findObjFromName(parentName);
				jointParent = dynamic_cast<Joint*>(obj);
			}

			spawnJoint(jointName, jointRot, jointTrans, jointParent);
		}
		cout << "Diagnostic Info: " << endl;
		cout << " - joints: " << scene.size() << endl;
	}
	in.close();
}

void Joint::draw() {

	//   get the current transformation matrix for this object
   //
	glm::mat4 m = getMatrix();

	//   push the current stack matrix and multiply by this object's
	//   matrix. now all vertices dran will be transformed by this matrix
	//
	ofPushMatrix();
	ofMultMatrix(m);
	ofDrawSphere(radius);
	ofPopMatrix();

	if (parent != NULL) { // Draw a bone connecting joint to its parent
		ofPushMatrix();

		glm::vec3 boneRot = { 0, 1, 0 }; // Default for OF
		glm::vec3 boneToParent = parent->getPosition() - getPosition();
		float length = glm::length(boneToParent);
		glm::mat4 rotationMatrix = rotateToVector(glm::normalize(boneRot), glm::normalize(boneToParent));

		glm::mat4 translationMatrix = glm::translate(getPosition());
		glm::mat4 offsetMiddleMatrix = glm::translate(glm::vec3(0, length / 2, 0));
		ofMultMatrix(translationMatrix * rotationMatrix * offsetMiddleMatrix);

		ofSetColor(diffuseColor);
		ofDrawCone(defaultRadius/2, length - 2*radius);
		ofPopMatrix();
	}

	// draw axis
	//
	ofApp::drawAxis(m, 1.5);

}

// IK Stuff
ofParameter<float> IKArm::learningRate{ "Learning rate", 100, 0, 1000 };
ofParameter<float> IKArm::deltaRotation{ "Delta rotation", 5, 0, 50 };
ofParameter<float> IKArm::distThreshold{ "Distance threshold", 0.3, 0, 10 };

// Test where the endjoint goes after a series of rotations starting from current joint rotations
glm::vec3 IKArm::simulateRotations(vector<float> jointAngles) {
	glm::mat4 sim(1.0f);
	for(int i = 0; i < joints.size(); i++) {
		auto joint = joints[i];
		auto angle = jointAngles[i];

		glm::vec3 rotationCopy = joint->rotation;
		rotationCopy = joint->lockedAxis * angle;
		glm::mat4 changedRotation = glm::eulerAngleYXZ(glm::radians(rotationCopy.y), glm::radians(rotationCopy.x), glm::radians(rotationCopy.z));
		//glm::mat4 changedRotation = joint->getRotateMatrix();

		// get the local transformations + pivot
		//
		glm::mat4 scale = joint->getScaleMatrix();
		glm::mat4 trans = joint->getTranslateMatrix();

		// handle pivot point  (rotate around a point that is not the object's center)
		//
		glm::mat4 pre = glm::translate(glm::mat4(1.0), glm::vec3(-joint->pivot.x, -joint->pivot.y, -joint->pivot.z));
		glm::mat4 post = glm::translate(glm::mat4(1.0), glm::vec3(joint->pivot.x, joint->pivot.y, joint->pivot.z));



		sim *= (trans * post * changedRotation * pre * scale);

		//sim *= joint->getLocalMatrix();
	}

	return sim * glm::vec4(0.0, 0.0, 0.0, 1.0);
}

// Return how close we are, using given joint angles - will be used as error function to minimize for gradient descent
float IKArm::distanceToTarget(vector<float> jointAngles) {
	glm::vec3 pt = simulateRotations(jointAngles); // End joints position after applying given angles
	return glm::distance(pt, target->getPosition());
}

// Computes gradient for a particular joint
// ( ( F(x + deltaRotation) - F(x) ) / deltaRotation ), where F is our error function
float IKArm::gradient(vector<float> jointAngles, int jointIndex) {
	float fX = distanceToTarget(jointAngles);
	jointAngles[jointIndex] += deltaRotation;
	float fXPlusDelta = distanceToTarget(jointAngles);
	float gradient = (fXPlusDelta - fX) / deltaRotation;
	return gradient;
}

// Move the arm towards the set target
void IKArm::moveTowardsTarget() {
	/*cout << "Positions:" << endl;
	for (auto joint : joints) cout << joint->getPosition() << endl;*/
	vector<float> angles = getAngles();
	/*cout << "Angles:" << endl;
	for(auto angle : angles) cout << angle << endl;*/

	/*cout << "Target Position:" << endl;
	cout << target->getPosition() << endl;*/
	float dist = distanceToTarget(angles);
	//cout << "Dist: " << dist << endl;
	if (dist < distThreshold) return; // Stop moving if we're close enough

	// Loop over all joints
	/*cout << "Angles before:" << endl;
	for(auto angle : angles) cout << angle << endl;*/
	for (int i = joints.size()-1; i >= 0; i--) {
		// Update the angles of each joint according to their individual gradients
		float individualGradient = gradient(angles, i);
		angles[i] -= learningRate * individualGradient;
		if (distanceToTarget(angles) < distThreshold) return; // Stop moving if we're close enough
	}
	/*cout << "Angles after:" << endl;
	for (auto angle : angles) cout << angle << endl;*/
	setAngles(angles); // Apply calculated rotation angles to joints
}

// Spawn IK arm and target
void ofApp::startIK() {
	cout << "Starting Inverse Kinematics" << endl;

	clearScene();

	glm::vec3 rot = { 0, 0, 0 };
	glm::vec3 trans = { 0, 0, 0 };
	glm::vec3 pos = { 0, -1.5, 0 }; 
	Joint* joint1 = new Joint("baseJoint", pos, rot, trans);
	pos = { .01, 2, 0 };
	Joint* joint2 = new Joint("midJoint", pos, rot, trans, joint1);
	pos = { 1, 3, 0 };
	Joint* joint3 = new Joint("endJoint", pos, rot, trans, joint2);
	Joint* joint4 = new Joint("endJoint", pos, rot, trans, joint3);
	vector<Joint*> joints = { joint1, joint2, joint3, joint4 };

	pos = { 0, 6, 0 };
	Joint* target = new Joint("target", pos, rot, trans);
	target->diffuseColor = ofColor::blue;
	IKArm* ikArm = new IKArm(joints, target);
	scene.push_back(target);
	scene.push_back(ikArm);
	for (auto joint : joints) scene.push_back(joint);
}


// Keyframing animation stuff
ofParameter<float> Animation::lengthInSeconds{ "Animation length (s)", 1, 0.1, 10 };

void Animation::start() {
	if (keyFrames.size() >= 2) {
		paused = false;
		hasReachedKeyFrame = false;
		timeAtLastKeyFrame = ofGetElapsedTimef();
		currentFrameIdx = 0;
		nextFrameIdx = 1;
		applyStartKeyFrame();
	}
}

void Animation::reset() {
	keyFrames.clear();
	paused = true;
	hasReachedKeyFrame = false;
}

void Animation::togglePause() {
	paused = !paused;
}


// Interpolate between keyframes to create an animation using the SceneObjects in each keyframe
void Animation::update() {
	if (!paused && keyFrames.size() >= 2) {
		animate();
		if (hasReachedKeyFrame) {
			hasReachedKeyFrame = false;
			if (currentFrameIdx == keyFrames.size() - 1) nextFrameIdx = 0;
			else nextFrameIdx = currentFrameIdx + 1;
			currentFrameIdx = nextFrameIdx;
		}
	}
}

void Animation::applyStartKeyFrame() {
	KeyFrame startKeyFrame = keyFrames[0];
	for (int i = 0; i < liveScene->size(); i++) {
		auto liveObj = (*liveScene)[i];
		glm::vec3 startPosition = startKeyFrame.scene[i].position;
		glm::vec3 startRotation = startKeyFrame.scene[i].rotation;
		liveObj->position = startPosition;
		liveObj->rotation = startRotation;
	}
}

void Animation::animate() {
	KeyFrame lastKeyFrame = keyFrames[currentFrameIdx];
	KeyFrame nextKeyFrame = keyFrames[nextFrameIdx];
	for (int i = 0; i < liveScene->size(); i++) {
		auto liveObj = (*liveScene)[i];
		//auto lastKeyFrameObj = lastKeyFrame.scene[i];
		auto nextKeyFrameObj = nextKeyFrame.scene[i];
		glm::vec3 livePosition = liveObj->position;
		glm::vec3 liveRotation = liveObj->rotation;
		/*glm::vec3 lastPosition = lastKeyFrame.scene[i].position;
		glm::vec3 lastRotation = lastKeyFrame.scene[i].rotation;*/
		glm::vec3 nextPosition = nextKeyFrame.scene[i].position;
		glm::vec3 nextRotation = nextKeyFrame.scene[i].rotation;

		float interp = getTimeSinceLastKeyFrame() / getTimePerKeyFrame();
		glm::vec3 interpPosition = lerp(livePosition, nextPosition, interp);
		glm::vec3 interpRotation = lerp(liveRotation, nextRotation, interp);
		liveObj->position = interpPosition;
		liveObj->rotation = interpRotation;
		
		if (interp >= 1) {
			hasReachedKeyFrame = true;
			timeAtLastKeyFrame = ofGetElapsedTimef();
		}
	}
}

void Animation::addFrameFromScene() {
	KeyFrame newFrame(*liveScene);
	keyFrames.push_back(newFrame);
	cout << "Added KeyFrame #" << keyFrames.size() << endl;
}

void ofApp::handleKeyFrameSave() {
	if(animation == nullptr) animation = new Animation(&scene);
	else animation->liveScene = &scene;
	animation->addFrameFromScene();
}

void ofApp::handleStartAnimation() {
	animation->start();
}

void ofApp::handleToggleAnimationPause() {
	animation->togglePause();
}
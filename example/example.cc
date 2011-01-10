#include <iostream>

#include "Model.h"
#include "Dynamics.h"

#include "mathutils.h" // required for Xtrans

int main (int argc, char* argv[]) {
	Model* model = NULL;

	unsigned int body_a_id, body_b_id, body_c_id;
	Body body_a, body_b, body_c;
	Joint joint_a, joint_b, joint_c;

	model = new Model();
	model->Init();

	body_a = Body (1., Vector3d (0.5, 0., 0.0), Vector3d (1., 1., 1.));
		joint_a = Joint(
		JointTypeRevolute,
		Vector3d (0., 0., 1.)
	);
	
	body_a_id = model->AddBody(0, Xtrans(Vector3d(0., 0., 0.)), joint_a, body_a);
	
	body_b = Body (1., Vector3d (0., 0.5, 0.), Vector3d (1., 1., 1.));
		joint_b = Joint (
		JointTypeRevolute,
		Vector3d (0., 0., 1.)
	);
	
	body_b_id = model->AddBody(body_a_id, Xtrans(Vector3d(1., 0., 0.)), joint_b, body_b);
	
	body_c = Body (0., Vector3d (0.5, 0., 0.), Vector3d (1., 1., 1.));
		joint_c = Joint (
		JointTypeRevolute,
		Vector3d (0., 0., 1.)
	);
	
	body_c_id = model->AddBody(body_b_id, Xtrans(Vector3d(0., 1., 0.)), joint_c, body_c);

	cmlVector Q(3);
	cmlVector QDot(3);
	cmlVector QDDot(3);
	cmlVector Tau(3);

 	ForwardDynamics (*model, Q, QDot, Tau, QDDot);

	std::cout << QDDot << std::endl;

 	return 0;
}


#include <UnitTest++.h>

#include <iostream>

#include "mathutils.h"
#include "Logging.h"

#include "Model.h"
#include "Dynamics_stdvec.h"

using namespace std;
using namespace SpatialAlgebra;

const double TEST_PREC = 1.0e-14;

struct ModelFixture {
	ModelFixture () {
		ClearLogOutput();
		model = new Model;
		model->Init();
		model->gravity.set (0., -9.81, 0.);
	}
	~ModelFixture () {
		delete model;
	}
	Model *model;
};

TEST_FIXTURE(ModelFixture, TestInit) {
	CHECK_EQUAL (1, model->lambda.size());

	CHECK_EQUAL (1, model->q.size());
	CHECK_EQUAL (1, model->qdot.size());
	CHECK_EQUAL (1, model->qddot.size());
	CHECK_EQUAL (1, model->tau.size());
	CHECK_EQUAL (1, model->v.size());
	CHECK_EQUAL (1, model->a.size());
	
	CHECK_EQUAL (1, model->mJoints.size());
	CHECK_EQUAL (1, model->S.size());

	CHECK_EQUAL (1, model->c.size());
	CHECK_EQUAL (1, model->IA.size());
	CHECK_EQUAL (1, model->pA.size());
	CHECK_EQUAL (1, model->U.size());
	CHECK_EQUAL (1, model->d.size());
	CHECK_EQUAL (1, model->u.size());
	CHECK_EQUAL (1, model->f_ext.size());
	
	CHECK_EQUAL (1, model->X_lambda.size());
	CHECK_EQUAL (1, model->X_base.size());
	CHECK_EQUAL (1, model->mBodies.size());
}

TEST_FIXTURE(ModelFixture, TestAddBodyDimensions) {
	Body body;
	Joint joint (
			JointTypeRevolute,
			Vector3d(0., 0., 1.)
			);

	unsigned int body_id = 0;
	body_id = model->AddBody(0, Xtrans(Vector3d(0., 0., 0.)), joint, body); 

	CHECK_EQUAL (1, body_id);
	CHECK_EQUAL (2, model->lambda.size());

	CHECK_EQUAL (2, model->q.size());
	CHECK_EQUAL (2, model->qdot.size());
	CHECK_EQUAL (2, model->qddot.size());
	CHECK_EQUAL (2, model->tau.size());
	CHECK_EQUAL (2, model->v.size());
	CHECK_EQUAL (2, model->a.size());
	
	CHECK_EQUAL (2, model->mJoints.size());
	CHECK_EQUAL (2, model->S.size());

	CHECK_EQUAL (2, model->c.size());
	CHECK_EQUAL (2, model->IA.size());
	CHECK_EQUAL (2, model->pA.size());
	CHECK_EQUAL (2, model->U.size());
	CHECK_EQUAL (2, model->d.size());
	CHECK_EQUAL (2, model->u.size());
	CHECK_EQUAL (2, model->f_ext.size());

	SpatialVector spatial_zero;
	spatial_zero.zero();
	CHECK_EQUAL (spatial_zero, model->f_ext.at(0));
	CHECK_EQUAL (spatial_zero, model->f_ext.at(1));
	
	CHECK_EQUAL (2, model->X_lambda.size());
	CHECK_EQUAL (2, model->X_base.size());
	CHECK_EQUAL (2, model->mBodies.size());
}


/** \brief Tests whether the joint and body information stored in the Model are computed correctly 
 */
TEST_FIXTURE(ModelFixture, TestAddBodySpatialValues) {
	Body body;
	Joint joint (
		JointTypeRevolute,
		Vector3d(0., 0., 1.)
		);

	model->AddBody(0, Xtrans(Vector3d(0., 0., 0.)), joint, body); 

	SpatialVector spatial_joint_axis(0., 0., 1., 0., 0., 0.);
	CHECK_EQUAL (spatial_joint_axis, joint.mJointAxis);

	// \Todo: Dynamic properties
}

TEST_FIXTURE(ModelFixture, TestjcalcSimple) {
	Body body;
	Joint joint (
		JointTypeRevolute,
		Vector3d(0., 0., 1.)
		);

	model->AddBody(0, Xtrans(Vector3d(1., 0., 0.)), joint, body);

	SpatialMatrix X_j;
	SpatialVector S;
	SpatialVector v_j;
	SpatialVector c;

	jcalc (*model, 1, X_j, S, v_j, c, 0., 1.);

	SpatialMatrix test_matrix (
			1.,  0.,  0.,  0.,  0.,  0.,
			0.,  1.,  0.,  0.,  0.,  0.,
			0.,  0.,  1.,  0.,  0.,  0.,
			0.,  0.,  0.,  1.,  0.,  0.,
			0.,  0.,  0.,  0.,  1.,  0.,
			0.,  0.,  0.,  0.,  0.,  1.
			);
	SpatialVector test_vector (
			0., 0., 1., 0., 0., 0.
			);
	SpatialVector test_joint_axis (
			0., 0., 1., 0., 0., 0.
			);

	CHECK (SpatialMatrixCompareEpsilon (test_matrix, X_j, 1.0e-16));
	CHECK (SpatialVectorCompareEpsilon (test_vector, v_j, 1.0e-16));
	CHECK_EQUAL (test_joint_axis, S);

	jcalc (*model, 1, X_j, S, v_j, c, M_PI * 0.5, 1.);

	test_matrix.set (
			0.,  1.,  0.,  0.,  0.,  0.,
		 -1.,  0.,  0.,  0.,  0.,  0.,
			0.,  0.,  1.,  0.,  0.,  0.,
			0.,  0.,  0.,  0.,  1.,  0.,
			0.,  0.,  0., -1.,  0.,  0.,
			0.,  0.,  0.,  0.,  0.,  1.
			);

	CHECK (SpatialMatrixCompareEpsilon (test_matrix, X_j, TEST_PREC));
	CHECK (SpatialVectorCompareEpsilon (test_vector, v_j, TEST_PREC));
	CHECK_EQUAL (test_joint_axis, S);
}

TEST_FIXTURE(ModelFixture, TestCalcVelocitiesSimple) {
	Body body(1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));
	Joint joint (
			JointTypeRevolute,
			Vector3d (0., 0., 1.)
			);

	Body endeffector (1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));
	Joint fixed_joint (
			JointTypeFixed,
			Vector3d (0., 0., 0.)
			);

	model->AddBody(0, Xtrans(Vector3d(0., 0., 0.)), joint, body);
	model->AddBody(1, Xtrans(Vector3d(1., 0., 0.)), fixed_joint, endeffector);

	std::vector<double> Q;
	std::vector<double> QDot;
	std::vector<double> QDDot;
	std::vector<double> Tau;

	// Initialization of the input vectors
	Q.push_back(0.); Q.push_back(0.);
	QDot.push_back(0.); QDot.push_back(0.);
	QDDot.push_back(0.); QDDot.push_back(0.);
	Tau.push_back(0.); Tau.push_back(0.);

	QDot.at(0) = 1.;
	ForwardDynamics(*model, Q, QDot, Tau, QDDot);

	SpatialVector spatial_body_velocity (0., 0., 1., 0., 1., 0.);
	CHECK_EQUAL (spatial_body_velocity, model->v.at(2));
	// std::cout << LogOutput.str() << std::endl;
	ClearLogOutput();

	QDot.at(0) = -1.;
	ForwardDynamics(*model, Q, QDot, Tau, QDDot);

	spatial_body_velocity.set (0., 0., -1., 0., -1., 0.);
	CHECK_EQUAL (spatial_body_velocity, model->v.at(2));
	// std::cout << LogOutput.str() << std::endl;
}

TEST_FIXTURE(ModelFixture, TestCalcDynamicSingleChain) {
	Body body(1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));
	Joint joint (
			JointTypeRevolute,
			Vector3d (0., 0., 1.)
			);

	model->AddBody(0, Xtrans(Vector3d(0., 0., 0.)), joint, body);

	std::vector<double> Q;
	std::vector<double> QDot;
	std::vector<double> QDDot;
	std::vector<double> Tau;

	// Initialization of the input vectors
	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	ForwardDynamics(*model, Q, QDot, Tau, QDDot);

	int i;
	for (i = 0; i < QDDot.size(); i++) {
		LOG << "QDDot[" << i << "] = " << QDDot[i] << endl;
	}

	for (i = 0; i < model->a.size(); i++) {
		LOG << "a[" << i << "]     = " << model->a[i] << endl;
	}

	CHECK_EQUAL (-4.905, QDDot[0]);
}

TEST_FIXTURE(ModelFixture, TestCalcDynamicSpatialInertiaSingleChain) {
	// This function checks the value for a non-trivial spatial inertia
	Body body(1., Vector3d (1.5, 1., 1.), Vector3d (1., 2., 3.));
	Joint joint (
			JointTypeRevolute,
			Vector3d (0., 0., 1.)
			);

	model->AddBody(0, Xtrans(Vector3d(0., 0., 0.)), joint, body);

	std::vector<double> Q;
	std::vector<double> QDot;
	std::vector<double> QDDot;
	std::vector<double> Tau;

	// Initialization of the input vectors
	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	ForwardDynamics(*model, Q, QDot, Tau, QDDot);

	int i;
	for (i = 0; i < QDDot.size(); i++) {
		LOG << "QDDot[" << i << "] = " << QDDot[i] << endl;
	}

	for (i = 0; i < model->a.size(); i++) {
		LOG << "a[" << i << "]     = " << model->a[i] << endl;
	}

	CHECK_EQUAL (-2.3544, QDDot[0]);
}

TEST_FIXTURE(ModelFixture, TestCalcDynamicDoubleChain) {
	Body body_a (1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));
	Joint joint_a (
			JointTypeRevolute,
			Vector3d (0., 0., 1.)
			);

	model->AddBody(0, Xtrans(Vector3d(0., 0., 0.)), joint_a, body_a);

	Body body_b (1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));
	Joint joint_b (
			JointTypeRevolute,
			Vector3d (0., 0., 1.)
			);

	model->AddBody(1, Xtrans(Vector3d(1., 0., 0.)), joint_b, body_b);

	std::vector<double> Q;
	std::vector<double> QDot;
	std::vector<double> QDDot;
	std::vector<double> Tau;

	// Initialization of the input vectors
	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

//	cout << "--- Double Chain ---" << endl;

	ForwardDynamics(*model, Q, QDot, Tau, QDDot);

	int i;
	for (i = 0; i < QDDot.size(); i++) {
		LOG << "QDDot[" << i << "] = " << QDDot[i] << endl;
	}

	for (i = 0; i < model->a.size(); i++) {
		LOG << "a[" << i << "]     = " << model->a[i] << endl;
	}

	//	cout << LogOutput.str() << endl;
	
	CHECK_CLOSE (-5.88600000000000E+00, QDDot[0], TEST_PREC);
	CHECK_CLOSE ( 3.92400000000000E+00, QDDot[1], TEST_PREC);
}

TEST_FIXTURE(ModelFixture, TestCalcDynamicTripleChain) {
	Body body_a (1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));
	Joint joint_a (
			JointTypeRevolute,
			Vector3d (0., 0., 1.)
			);

	model->AddBody(0, Xtrans(Vector3d(0., 0., 0.)), joint_a, body_a);

	Body body_b (1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));
	Joint joint_b (
			JointTypeRevolute,
			Vector3d (0., 0., 1.)
			);

	model->AddBody(1, Xtrans(Vector3d(1., 0., 0.)), joint_b, body_b);

	Body body_c (1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));
	Joint joint_c (
			JointTypeRevolute,
			Vector3d (0., 0., 1.)
			);

	model->AddBody(2, Xtrans(Vector3d(1., 0., 0.)), joint_c, body_c);

	std::vector<double> Q;
	std::vector<double> QDot;
	std::vector<double> QDDot;
	std::vector<double> Tau;

	// Initialization of the input vectors
	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	// cout << "--- Triple Chain ---" << endl;

	ForwardDynamics(*model, Q, QDot, Tau, QDDot);

	int i;
	for (i = 0; i < QDDot.size(); i++) {
		LOG << "QDDot[" << i << "] = " << QDDot[i] << endl;
	}

	for (i = 0; i < model->a.size(); i++) {
		LOG << "a[" << i << "]     = " << model->a[i] << endl;
	}

	// cout << LogOutput.str() << endl;

	CHECK_CLOSE (-6.03692307692308E+00, QDDot[0], TEST_PREC);
	CHECK_CLOSE ( 3.77307692307692E+00, QDDot[1], TEST_PREC);
	CHECK_CLOSE ( 1.50923076923077E+00, QDDot[2], TEST_PREC);
}

TEST_FIXTURE(ModelFixture, TestCalcDynamicDoubleChain3D) {
	Body body_a (1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));
	Joint joint_a (
			JointTypeRevolute,
			Vector3d (0., 0., 1.)
			);

	model->AddBody(0, Xtrans(Vector3d(0., 0., 0.)), joint_a, body_a);

	Body body_b (1., Vector3d (0., 1., 0.), Vector3d (1., 1., 1.));
	Joint joint_b (
			JointTypeRevolute,
			Vector3d (0., 1., 0.)
			);

	model->AddBody(1, Xtrans(Vector3d(1., 0., 0.)), joint_b, body_b);

	std::vector<double> Q;
	std::vector<double> QDot;
	std::vector<double> QDDot;
	std::vector<double> Tau;

	// Initialization of the input vectors
	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	// cout << "--- Double Chain 3D ---" << endl;

	ForwardDynamics(*model, Q, QDot, Tau, QDDot);

	int i;
	for (i = 0; i < QDDot.size(); i++) {
		LOG << "QDDot[" << i << "] = " << QDDot[i] << endl;
	}

	for (i = 0; i < model->a.size(); i++) {
		LOG << "a[" << i << "]     = " << model->a[i] << endl;
	}

	// cout << LogOutput.str() << endl;

	CHECK_CLOSE (-3.92400000000000E+00, QDDot[0], TEST_PREC);
	CHECK_CLOSE ( 0.00000000000000E+00, QDDot[1], TEST_PREC);
}

TEST_FIXTURE(ModelFixture, TestCalcDynamicSimpleTree3D) {
	Body body_a (1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));
	Joint joint_a (
			JointTypeRevolute,
			Vector3d (0., 0., 1.)
			);

	model->AddBody(0, Xtrans(Vector3d(0., 0., 0.)), joint_a, body_a);

	Body body_b1 (1., Vector3d (0., 1., 0.), Vector3d (1., 1., 1.));
	Joint joint_b1 (
			JointTypeRevolute,
			Vector3d (0., 1., 0.)
			);

	model->AddBody(1, Xtrans(Vector3d(1., 0., 0.)), joint_b1, body_b1);

	Body body_c1 (1., Vector3d (0., 0., 1.), Vector3d (1., 1., 1.));
	Joint joint_c1 (
			JointTypeRevolute,
			Vector3d (1., 0., 0.)
			);

	model->AddBody(2, Xtrans(Vector3d(0., 1., 0.)), joint_c1, body_c1);

	Body body_b2 (1., Vector3d (0., 1., 0.), Vector3d (1., 1., 1.));
	Joint joint_b2 (
			JointTypeRevolute,
			Vector3d (0., 1., 0.)
			);

	model->AddBody(1, Xtrans(Vector3d(-0.5, 0., 0.)), joint_b2, body_b2);

	Body body_c2 (1., Vector3d (0., 0., 1.), Vector3d (1., 1., 1.));
	Joint joint_c2 (
			JointTypeRevolute,
			Vector3d (1., 0., 0.)
			);

	model->AddBody(4, Xtrans(Vector3d(0., -0.5, 0.)), joint_c2, body_c2);

	std::vector<double> Q;
	std::vector<double> QDot;
	std::vector<double> QDDot;
	std::vector<double> Tau;

	// Initialization of the input vectors
	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	Q.push_back(0.);
	QDot.push_back(0.);
	QDDot.push_back(0.);
	Tau.push_back(0.);

	// cout << "--- SimpleTree ---" << endl;

	ForwardDynamics(*model, Q, QDot, Tau, QDDot);

	int i;
	for (i = 0; i < QDDot.size(); i++) {
		LOG << "QDDot[" << i << "] = " << QDDot[i] << endl;
	}

	for (i = 0; i < model->a.size(); i++) {
		LOG << "a[" << i << "]     = " << model->a[i] << endl;
	}

	// cout << LogOutput.str() << endl;

	CHECK_CLOSE (-1.60319066147860E+00, QDDot[0], TEST_PREC);
	CHECK_CLOSE (-5.34396887159533E-01, QDDot[1], TEST_PREC);
	CHECK_CLOSE ( 4.10340466926070E+00, QDDot[2], TEST_PREC);
	CHECK_CLOSE ( 2.67198443579767E-01, QDDot[3], TEST_PREC);
	CHECK_CLOSE ( 5.30579766536965E+00, QDDot[4], TEST_PREC);
}

TEST_FIXTURE(ModelFixture, TestCalcDynamicFloatingBaseSimple) {
	model->floating_base = true;

	Body base(1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));

	model->SetFloatingBody(base);

	std::vector<double> Q (0, 0.);
	std::vector<double> QDot (0, 0.);
	std::vector<double> QDDot (0, 0.);
	std::vector<double> Tau (0, 0.);

	Vector3d pos_B(0., 0., 0.);
	Vector3d rot_B(0., 0., 0.);

	SpatialMatrix X_B (XtransRotZYXEuler(pos_B, rot_B));
	SpatialVector v_B(0., 0., 0., 0., 0., 0.);
	SpatialVector f_B(0., 0., 0., 0., 0., 0.);
	SpatialVector a_B(0., 0., 0., 0., 0., 0.);

	ForwardDynamicsFloatingBase(*model, Q, QDot, Tau, X_B, v_B, f_B, a_B, QDDot);

	int i;
	for (i = 0; i < QDDot.size(); i++) {
		LOG << "QDDot[" << i << "] = " << QDDot.at(i) << endl;
	}

	for (i = 0; i < model->a.size(); i++) {
		LOG << "a[" << i << "]     = " << model->a.at(i) << endl;
	}

//	std::cout << LogOutput.str() << std::endl;

	CHECK_CLOSE ( 0.0000, a_B[0], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_B[1], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_B[2], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_B[3], TEST_PREC);
	CHECK_CLOSE (-9.8100, a_B[4], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_B[5], TEST_PREC);

	// We rotate the base... let's see what happens...
	rot_B[0] = 0.8;
	X_B = XtransRotZYXEuler(pos_B, rot_B);
	ForwardDynamicsFloatingBase(*model, Q, QDot, Tau, X_B, v_B, f_B, a_B, QDDot);
	SpatialVector a_world = X_B.inverse() * a_B;

	for (i = 0; i < QDDot.size(); i++) {
		LOG << "QDDot[" << i << "] = " << QDDot.at(i) << endl;
	}

	for (i = 0; i < model->a.size(); i++) {
		LOG << "a[" << i << "]     = " << model->a.at(i) << endl;
	}

//	std::cout << LogOutput.str() << std::endl;

	CHECK_CLOSE ( 0.0000, a_world[0], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_world[1], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_world[2], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_world[3], TEST_PREC);
	CHECK_CLOSE (-9.8100, a_world[4], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_world[5], TEST_PREC);
}

TEST_FIXTURE(ModelFixture, TestCalcDynamicFloatingBaseDouble) {
	model->floating_base = true;

	// floating base
	Body base(1., Vector3d (1., 0., 0.), Vector3d (1., 1., 1.));
	model->SetFloatingBody(base);

	// body_a
	Body body_a (1., Vector3d (1., 0., 0), Vector3d (1., 1., 1.));
	Joint joint_a (
			JointTypeRevolute,
			Vector3d (0., 0., 1.)
			);

	model->AddBody(0, Xtrans(Vector3d(2., 0., 0.)), joint_a, body_a);

	std::vector<double> Q (1, 0.);
	std::vector<double> QDot (1, 0.);
	std::vector<double> QDDot (1, 0.);
	std::vector<double> Tau (1, 0.);

	Vector3d pos_B(0., 0., 0.);
	Vector3d rot_B(0., 0., 0.);

	SpatialMatrix X_B (XtransRotZYXEuler(pos_B, rot_B));
	SpatialVector v_B(0., 0., 0., 0., 0., 0.);
	SpatialVector f_B(0., 0., 0., 0., 0., 0.);
	SpatialVector a_B(0., 0., 0., 0., 0., 0.);
	SpatialVector a_world(0., 0., 0., 0., 0., 0.);

	ForwardDynamicsFloatingBase(*model, Q, QDot, Tau, X_B, v_B, f_B, a_B, QDDot);

	int i;
	for (i = 0; i < QDDot.size(); i++) {
		LOG << "QDDot[" << i << "] = " << QDDot.at(i) << endl;
	}

	for (i = 0; i < model->a.size(); i++) {
		LOG << "a[" << i << "]     = " << model->a.at(i) << endl;
	}

//	std::cout << LogOutput.str() << std::endl;

	CHECK_CLOSE ( 0.0000, a_B[0], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_B[1], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_B[2], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_B[3], TEST_PREC);
	CHECK_CLOSE (-9.8100, a_B[4], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_B[5], TEST_PREC);
	CHECK_CLOSE ( 0.0000, QDDot[0], TEST_PREC);

	// We rotate the base... let's see what happens...
	rot_B[0] = 0.8;
	X_B = XtransRotZYXEuler(pos_B, rot_B);
	ForwardDynamicsFloatingBase(*model, Q, QDot, Tau, X_B, v_B, f_B, a_B, QDDot);
	a_world = X_B.inverse() * a_B;

	for (i = 0; i < QDDot.size(); i++) {
		LOG << "QDDot[" << i << "] = " << QDDot.at(i) << endl;
	}

	for (i = 0; i < model->a.size(); i++) {
		LOG << "a[" << i << "]     = " << model->a.at(i) << endl;
	}

//	std::cout << LogOutput.str() << std::endl;

	CHECK_CLOSE ( 0.0000, a_world[0], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_world[1], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_world[2], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_world[3], TEST_PREC);
	CHECK_CLOSE (-9.8100, a_world[4], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_world[5], TEST_PREC);
	CHECK_CLOSE ( 0.0000, QDDot[0], TEST_PREC);

	// We apply a torqe let's see what happens...
	rot_B[0] = 0.0;
	X_B = XtransRotZYXEuler(pos_B, rot_B);

	Tau[0] = 1.;

	ForwardDynamicsFloatingBase(*model, Q, QDot, Tau, X_B, v_B, f_B, a_B, QDDot);
	a_world = X_B.inverse() * a_B;

	for (i = 0; i < QDDot.size(); i++) {
		LOG << "QDDot[" << i << "] = " << QDDot.at(i) << endl;
	}

	for (i = 0; i < model->a.size(); i++) {
		LOG << "a[" << i << "]     = " << model->a.at(i) << endl;
	}

//	std::cout << LogOutput.str() << std::endl;

	CHECK_CLOSE ( 0.0000, a_world[0], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_world[1], TEST_PREC);
	CHECK_CLOSE (-1.0000, a_world[2], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_world[3], TEST_PREC);
	CHECK_CLOSE (-8.8100, a_world[4], TEST_PREC);
	CHECK_CLOSE ( 0.0000, a_world[5], TEST_PREC);
	CHECK_CLOSE ( 2.0000, QDDot[0],   TEST_PREC);
}

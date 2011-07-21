#ifndef _MATHWRAPPER_H
#define _MATHWRAPPER_H

#include "rbdlconfig.h"

// #define GINAC_MATH

#ifdef GINAC_MATH
	#define RBDL_USE_SIMPLE_MATH
	
	#include "ginac.h"
  #include "SimpleMath.h"
	#include <vector>

	using namespace GiNaC;

	typedef GiNaC::ex value_type;

	typedef SimpleMath::Fixed::Matrix<value_type, 3,1> Vector3d;
	typedef SimpleMath::Fixed::Matrix<value_type, 3,3> Matrix3d;

	namespace SpatialAlgebra {
		typedef SimpleMath::Fixed::Matrix<value_type, 6,1> SpatialVector;
		typedef SimpleMath::Fixed::Matrix<value_type, 6,6> SpatialMatrix;
	}

	typedef SimpleMath::Dynamic::Matrix<value_type> MatrixNd;
	typedef SimpleMath::Dynamic::Matrix<value_type> VectorNd;
#elif defined RBDL_USE_SIMPLE_MATH
  #include "SimpleMath.h"
	#include <vector>

	typedef double value_type;

	typedef SimpleMath::Fixed::Matrix<double, 3,1> Vector3d;
	typedef SimpleMath::Fixed::Matrix<double, 3,3> Matrix3d;

	namespace SpatialAlgebra {
		typedef SimpleMath::Fixed::Matrix<double, 6,1> SpatialVector;
		typedef SimpleMath::Fixed::Matrix<double, 6,6> SpatialMatrix;
	}

	typedef SimpleMath::Dynamic::Matrix<double> MatrixNd;
	typedef SimpleMath::Dynamic::Matrix<double> VectorNd;
#else
	#define EIGEN_DEFAULT_TO_ROW_MAJOR
	#define EIGEN_MATRIX_PLUGIN "MatrixAddons.h"

	#include "Eigen/Dense"
	#include "Eigen/StdVector"

	typedef double value_type;

	typedef Eigen::Matrix< double, 3, 1> Vector3d;
	typedef Eigen::Matrix< double, 3, 3> Matrix3d;

	typedef Eigen::VectorXd VectorNd;
	typedef Eigen::MatrixXd MatrixNd;

	namespace SpatialAlgebra {
		typedef Eigen::Matrix< double, 6, 1> SpatialVector;
		typedef Eigen::Matrix< double, 6, 6> SpatialMatrix;
	}

	inline EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(SpatialAlgebra::SpatialVector)
	inline EIGEN_DEFINE_STL_VECTOR_SPECIALIZATION(SpatialAlgebra::SpatialMatrix)
#endif

#include "SpatialAlgebraOperators.h"

#endif /* _MATHWRAPPER_H */

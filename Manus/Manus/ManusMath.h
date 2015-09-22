#pragma once
class ManusMath
{
public:
	/*! \brief Convert a Quaternion to Euler angles.
	*
	*  Returns the Quaternion as Yaw, Pitch and Roll angles
	*  relative to the Earth's gravity.
	*
	*  \param euler Output variable to receive the Euler angles.
	*  \param quaternion The quaternion to convert.
	*/
	static int GetEuler(GLOVE_VECTOR* euler, const GLOVE_QUATERNION* quaternion);

	/*! \brief Remove gravity from acceleration vector.
	*
	*  Returns the Acceleration as a vector independent from
	*  the Earth's gravity.
	*
	*  \param linear Output vector to receive the linear acceleration.
	*  \param acceleation The acceleration vector to convert.
	*/
	static int GetLinearAcceleration(GLOVE_VECTOR* linear, const GLOVE_VECTOR* acceleration, const GLOVE_VECTOR* gravity);

	/*! \brief Return gravity vector from the Quaternion.
	*
	*  Returns an estimation of the Earth's gravity vector.
	*
	*  \param gravity Output vector to receive the gravity vector.
	*  \param quaternion The quaternion to base the gravity vector on.
	*/
	static int GetGravity(GLOVE_VECTOR* gravity, const GLOVE_QUATERNION* quaternion);


	

private:
	ManusMath();
};


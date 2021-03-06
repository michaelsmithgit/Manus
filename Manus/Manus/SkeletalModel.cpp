#include "stdafx.h"
#include "SkeletalModel.h"
#include "FbxMemStream.h"
#include "resource.h"
#include "ManusMath.h"

const char* s_bone_names[GLOVE_FINGERS][3] = {
	{ "ThumbFingerBone004", "ThumbFingerBone005", "ThumbFingerBone003" },
	{ "IndexFingerBone003", "IndexFingerBone005", "IndexFingerBone004" },
	{ "MiddleFingerBone003", "MiddleFingerBone004", "MiddleFingerBone005" },
	{ "RingFingerBone004", "RingFingerBone005", "RingFingerBone003" },
	{ "PinkFingerBone004", "PinkFingerBone005", "PinkFingerBone003" }
};

GLOVE_POSE SkeletalModel::ToGlovePose(FbxAMatrix mat)
{
	GLOVE_POSE pose;

	// Apply the orientation of the hand to the transformation matrix
	FbxQuaternion orient;

	/*
	if (temp_hand == GLOVE_RIGHT)
		//orient = FbxQuaternion(-temp_data.Quaternion.y, temp_data.Quaternion.z, -temp_data.Quaternion.x, temp_data.Quaternion.w);
		orient = FbxQuaternion(-temp_data.Quaternion.y, temp_data.Quaternion.z, temp_data.Quaternion.x, -temp_data.Quaternion.w);
	else
		orient = FbxQuaternion(temp_data.Quaternion.y, temp_data.Quaternion.z, temp_data.Quaternion.x, temp_data.Quaternion.w);
	*/

	orient = FbxQuaternion(temp_data.Quaternion.y, temp_data.Quaternion.z, temp_data.Quaternion.x, temp_data.Quaternion.w);

	FbxAMatrix orientMat;
	orientMat.SetQ(orient);
	orientMat *= mat;

	FbxQuaternion quat = orientMat.GetQ();

	pose.orientation.x = (float)quat.mData[0];
	pose.orientation.y = (float)quat.mData[1];
	pose.orientation.z = (float)quat.mData[2];
	pose.orientation.w = (float)quat.mData[3];

	FbxVector4 trans = orientMat.GetT();

	pose.position.x = (float)trans.mData[0];
	pose.position.y = (float)trans.mData[1];
	pose.position.z = (float)trans.mData[2];

	/*
	if (temp_hand == GLOVE_RIGHT)
		pose.position.x *= -1.0f;
	*/
	return pose;
}

// Creates an instance of the SDK manager
// and use the SDK manager to create a new scene
SkeletalModel::SkeletalModel()
{
}

SkeletalModel::~SkeletalModel()
{
	// Delete the FBX SDK manager. All the objects that have been allocated 
	// using the FBX SDK manager and that haven't been explicitly destroyed 
	// are automatically destroyed at the same time.
	if (m_sdk_manager)
		m_sdk_manager->Destroy();
}

bool SkeletalModel::InitializeScene()
{
	// Create the FBX SDK memory manager object.
	// The SDK Manager allocates and frees memory
	// for almost all the classes in the SDK.
	m_sdk_manager = FbxManager::Create();

	// Create an IOSettings object.
	FbxIOSettings* ios = FbxIOSettings::Create(m_sdk_manager, IOSROOT);
	m_sdk_manager->SetIOSettings(ios);

	// Get pointer and size to resource.
	HRSRC hRes = FindResource(GetModuleHandle(L"Manus.dll"), MAKEINTRESOURCE(IDR_FBX1), RT_RCDATA);
	HGLOBAL hMem = LoadResource(GetModuleHandle(L"Manus.dll"), hRes);
	DWORD dSize = SizeofResource(GetModuleHandle(L"Manus.dll"), hRes);
	void* pMem = LockResource(hMem);

	// Create an importer and initialize the importer.
	FbxImporter* importer = FbxImporter::Create(m_sdk_manager, "");
	FbxMemStream mem_stream(m_sdk_manager, pMem, dSize);
	if (!importer->Initialize(&mem_stream, nullptr, -1, m_sdk_manager->GetIOSettings()))
	{
		FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
		FBXSDK_printf("Error returned: %s\n\n", importer->GetStatus().GetErrorString());
		return false;
	}

	// Create a new scene so it can be populated by the imported file.
	m_scene = FbxScene::Create(m_sdk_manager, "HandModel");

	// Import the contents of the file into the scene.
	importer->Import(m_scene);

	// Get the hand node
	m_palm_node = m_scene->FindNodeByName("Palm");

	// Get the palm bone
	m_palm_bone = m_scene->FindNodeByName("Palm bone");

	// Get the bones for each finger.
	for (int i = 0; i < GLOVE_FINGERS; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			// Get the node for the finger
			m_bone_nodes[i][j] = m_scene->FindNodeByName(s_bone_names[i][j]);
		}
	}

	// The file has been imported; we can get rid of the importer.
	importer->Destroy();

	return true;
}




bool SkeletalModel::Simulate(const GLOVE_DATA data, GLOVE_SKELETAL* model, GLOVE_HAND hand)
{
	// Get the animation evaluator for this scene
	FbxAnimEvaluator* eval = m_scene->GetAnimationEvaluator();
	FbxTime normalizedAmount;
	double timeFactor = 1.66;

	temp_data = data;
	temp_hand = hand;

	// Swapping as in ToGlovePose;
	temp_quaternion.x = data.Quaternion.y;
	temp_quaternion.y = data.Quaternion.z;
	temp_quaternion.z = data.Quaternion.x;
	temp_quaternion.w = data.Quaternion.w;
	
	// Rotation to match Unity
	GLOVE_QUATERNION rotation;
	rotation.x = -0.707f;
	rotation.y = 0;
	rotation.z = 0;
	rotation.w = 0.707f;
	temp_quaternion = ManusMath::QuaternionMultiply(temp_quaternion, rotation);
	
	// Set the pose of the palm
	model->palm.orientation = temp_quaternion;

	// Evaluate the animation for the thumb
	normalizedAmount.SetSecondDouble(data.Fingers[0] * timeFactor);
	model->thumb.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[0][0], normalizedAmount));
	model->thumb.proximal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[0][1], normalizedAmount));
	model->thumb.distal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[0][2], normalizedAmount));

	// Evaluate the animation for the index finger
	normalizedAmount.SetSecondDouble(data.Fingers[1] * timeFactor);
	model->index.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_palm_bone, normalizedAmount));
	model->index.proximal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[1][0], normalizedAmount));
	model->index.intermediate = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[1][1], normalizedAmount));
	model->index.distal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[1][2], normalizedAmount));

	// Evaluate the animation for the middle finger
	normalizedAmount.SetSecondDouble(data.Fingers[2] * timeFactor);
	model->middle.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_palm_bone, normalizedAmount));
	model->middle.proximal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[2][0], normalizedAmount));
	model->middle.intermediate = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[2][1], normalizedAmount));
	model->middle.distal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[2][2], normalizedAmount));

	// Evaluate the animation for the ring finger
	normalizedAmount.SetSecondDouble(data.Fingers[3] * timeFactor);
	model->ring.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_palm_bone, normalizedAmount));
	model->ring.proximal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[3][0], normalizedAmount));
	model->ring.intermediate = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[3][1], normalizedAmount));
	model->ring.distal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[3][2], normalizedAmount));

	// Evaluate the animation for the pink finger
	normalizedAmount.SetSecondDouble(data.Fingers[4] * timeFactor);
	model->pinky.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_palm_bone, normalizedAmount));
	model->pinky.proximal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[4][0], normalizedAmount));
	model->pinky.intermediate = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[4][1], normalizedAmount));
	model->pinky.distal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[4][2], normalizedAmount));

	return true;
}

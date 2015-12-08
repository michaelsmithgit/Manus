#include "stdafx.h"
#include "SkeletalModel.h"
#include "FbxMemStream.h"
#include "resource.h"
#include "ManusMath.h"

const char* s_bone_names[GLOVE_FINGERS][4] = {
	{ "Finger_00", "Finger_01", "Finger_02", "Finger_03" },
	{ "Finger_10", "Finger_11", "Finger_12", "Finger_13" },
	{ "Finger_20", "Finger_21", "Finger_22", "Finger_23" },
	{ "Finger_30", "Finger_31", "Finger_32", "Finger_33" },
	{ "Finger_40", "Finger_41", "Finger_42", "Finger_43" },
};


GLOVE_POSE SkeletalModel::ToGlovePose(FbxAMatrix mat, GLOVE_QUATERNION &Quat)
{
	GLOVE_POSE pose;

	// Apply the orientation of the hand to the transformation matrix
	FbxQuaternion orient = FbxQuaternion(Quat.x, Quat.y, Quat.z, Quat.w);
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
	// Arrays of size 2 for the left/right hand loop
	HRSRC hRes[2];
	HGLOBAL hMem[2];
	DWORD dSize[2];
	void* pMem[2];
	FbxImporter* importer[2];
	FbxMemStream* mem_stream[2];

	// Create the FBX SDK memory manager object.
	// The SDK Manager allocates and frees memory
	// for almost all the classes in the SDK.
	m_sdk_manager = FbxManager::Create();

	// Create an IOSettings object.
	FbxIOSettings* ios = FbxIOSettings::Create(m_sdk_manager, IOSROOT);
	m_sdk_manager->SetIOSettings(ios);



	for (int i = 0; i < 2; i++)
	{

		// Get pointer and size to resource.
		hRes[i] = FindResource(GetModuleHandle(L"Manus.dll"),
			i ? MAKEINTRESOURCE(IDR_FBX_RIGHT) : MAKEINTRESOURCE(IDR_FBX_LEFT),
			RT_RCDATA);
		hMem[i] = LoadResource(GetModuleHandle(L"Manus.dll"), hRes[i]);
		dSize[i] = SizeofResource(GetModuleHandle(L"Manus.dll"), hRes[i]);
		pMem[i] = LockResource(hMem[i]);

		// Create an importer and initialize the importer.
		importer[i] = FbxImporter::Create(m_sdk_manager, "");
		mem_stream[i] = new FbxMemStream(m_sdk_manager, pMem[i], dSize[i]);
		if (!importer[i]->Initialize(mem_stream[i], nullptr, -1, m_sdk_manager->GetIOSettings()))
		{
			FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
			FBXSDK_printf("Error returned: %s\n\n", importer[i]->GetStatus().GetErrorString());
			return false;
		}

		// Create a new scene so it can be populated by the imported file.
		m_scene[i] = FbxScene::Create(m_sdk_manager, i ? "Manus_Handv2_Right" : "Manus_Handv2_Left");

		// Import the contents of the file into the scene.
		importer[i]->Import(m_scene[i]);

		// The file has been imported; we can get rid of the importer.
		importer[i]->Destroy();

		// To put it in the loop, we're using new FbxMemStream, so we must also delete it.
		delete mem_stream[i];
	}


	// Get the bones for each finger.
	for (int i = 0; i < GLOVE_FINGERS; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			// Get the node for the finger
			m_bone_nodes[GLOVE_LEFT][i][j] = m_scene[GLOVE_LEFT]->FindNodeByName(s_bone_names[i][j]);
			m_bone_nodes[GLOVE_RIGHT][i][j] = m_scene[GLOVE_RIGHT]->FindNodeByName(s_bone_names[i][j]);
		}
	}



	return true;
}




bool SkeletalModel::Simulate(const GLOVE_DATA data, GLOVE_SKELETAL* model, GLOVE_HAND hand, bool OSVR_Compat)
{
	// Get the animation evaluator for this scene
	FbxAnimEvaluator* eval = m_scene[hand]->GetAnimationEvaluator();
	FbxTime normalizedAmount;
	double timeFactor = 10.0;
	GLOVE_QUATERNION Quat;
	

	if (OSVR_Compat)
	{
		// OSVR
		Quat.x = -data.Quaternion.y;
		Quat.y = data.Quaternion.z;
		Quat.z = -data.Quaternion.x;
		Quat.w = data.Quaternion.w;
	}
	else {
		// Manus
		Quat.x = data.Quaternion.y;
		Quat.y = data.Quaternion.z;
		Quat.z = data.Quaternion.x;
		Quat.w = data.Quaternion.w;
	}

	// Set the pose of the palm
	model->palm.orientation = Quat;

	// Evaluate the animation for the thumb
	normalizedAmount.SetSecondDouble(data.Fingers[0] * timeFactor);
	model->thumb.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][0][0], normalizedAmount), Quat );
	model->thumb.proximal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][0][1], normalizedAmount), Quat );
	model->thumb.intermediate = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][0][2], normalizedAmount), Quat );
	model->thumb.distal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][0][3], normalizedAmount), Quat );

	// Evaluate the animation for the index finger
	normalizedAmount.SetSecondDouble(data.Fingers[1] * timeFactor);
	model->index.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][1][0], normalizedAmount), Quat );
	model->index.proximal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][1][1], normalizedAmount), Quat );
	model->index.intermediate = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][1][2], normalizedAmount), Quat );
	model->index.distal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][1][3], normalizedAmount), Quat );

	// Evaluate the animation for the middle finger
	normalizedAmount.SetSecondDouble(data.Fingers[2] * timeFactor);
	model->middle.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][2][0], normalizedAmount), Quat );
	model->middle.proximal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][2][1], normalizedAmount), Quat );
	model->middle.intermediate = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][2][2], normalizedAmount), Quat );
	model->middle.distal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][2][3], normalizedAmount), Quat );

	// Evaluate the animation for the ring finger
	normalizedAmount.SetSecondDouble(data.Fingers[3] * timeFactor);
	model->ring.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][3][0], normalizedAmount), Quat );
	model->ring.proximal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][3][1], normalizedAmount), Quat );
	model->ring.intermediate = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][3][2], normalizedAmount), Quat );
	model->ring.distal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][3][3], normalizedAmount), Quat );

	// Evaluate the animation for the pink finger
	normalizedAmount.SetSecondDouble(data.Fingers[4] * timeFactor);
	model->pinky.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][4][0], normalizedAmount), Quat );
	model->pinky.proximal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][4][1], normalizedAmount), Quat );
	model->pinky.intermediate = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][4][2], normalizedAmount), Quat );
	model->pinky.distal = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[hand][4][3], normalizedAmount), Quat );

	return true;
}

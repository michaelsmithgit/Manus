#include "stdafx.h"
#include "SkeletalModel.h"
#include "FbxMemStream.h"
#include "resource.h"

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
	FbxQuaternion quat = mat.GetQ();

	pose.orientation.x = (float)quat.mData[0];
	pose.orientation.y = (float)quat.mData[1];
	pose.orientation.z = (float)quat.mData[2];
	pose.orientation.w = (float)quat.mData[3];

	FbxVector4 trans = mat.GetT();

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

	// Get the palm bone
	m_palm_node = m_scene->FindNodeByName("Palm bone");

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

bool SkeletalModel::Simulate(const GLOVE_STATE* state, GLOVE_SKELETAL* model)
{
	// Get the animation evaluator for this scene.
	FbxAnimEvaluator* eval = m_scene->GetAnimationEvaluator();
	FbxTime normalizedAmount;

	// Evaluate the animation for the thumb
	normalizedAmount.SetSecondDouble(state->data.Fingers[0]);
	for (int i = 0; i < 3; i++)
		*(&model->thumb.metacarpal + i) = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[0][i], normalizedAmount));

	// Evaluate the animation for the index finger
	normalizedAmount.SetSecondDouble(state->data.Fingers[1]);
	model->index.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_palm_node, normalizedAmount));
	for (int i = 0; i < 3; i++)
		*(&model->index.proximal + i) = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[0][i], normalizedAmount));

	// Evaluate the animation for the middle finger
	normalizedAmount.SetSecondDouble(state->data.Fingers[2]);
	model->middle.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_palm_node, normalizedAmount));
	for (int i = 0; i < 3; i++)
		*(&model->middle.proximal + i) = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[0][i], normalizedAmount));

	// Evaluate the animation for the ring finger
	normalizedAmount.SetSecondDouble(state->data.Fingers[3]);
	model->ring.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_palm_node, normalizedAmount));
	for (int i = 0; i < 3; i++)
		*(&model->ring.proximal + i) = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[0][i], normalizedAmount));

	// Evaluate the animation for the pink finger
	normalizedAmount.SetSecondDouble(state->data.Fingers[4]);
	model->pinky.metacarpal = ToGlovePose(eval->GetNodeGlobalTransform(m_palm_node, normalizedAmount));
	for (int i = 0; i < 3; i++)
		*(&model->pinky.proximal + i) = ToGlovePose(eval->GetNodeGlobalTransform(m_bone_nodes[0][i], normalizedAmount));

	return true;
}

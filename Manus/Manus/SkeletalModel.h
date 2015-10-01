/**
* Copyright (C) 2015 Manus Machina
*
* This file is part of the Manus SDK.
*
* Manus SDK is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Manus SDK is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with Manus SDK. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Manus.h"
#include "Glove.h"
#include <fbxsdk.h>

class SkeletalModel
{
private:
	FbxManager* m_sdk_manager;
	FbxScene* m_scene;
	FbxNode* m_palm_node;
	FbxNode* m_palm_bone;
	FbxNode* m_bone_nodes[GLOVE_FINGERS][3];

	GLOVE_POSE ToGlovePose(FbxAMatrix mat);

	// temporary states for the ToGlovePose function call
	GLOVE_HAND temp_hand;
	GLOVE_DATA temp_data;
	GLOVE_QUATERNION temp_quaternion;

public:
	SkeletalModel();
	~SkeletalModel();

	bool InitializeScene();
	bool Simulate(const GLOVE_DATA data, GLOVE_SKELETAL* model, GLOVE_HAND hand);
};
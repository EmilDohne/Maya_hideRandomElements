#include "HideElementsNode.h"
#include "Timer.h"

#include <maya/MPxLocatorNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnNumericData.h>
#include <maya/MGlobal.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MIntArray.h>
#include <maya/MUIntArray.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

#include <maya/MIOStream.h>
#include <maya/MRandom.h>

#include <time.h>
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_map>

//----------------------------------------------------------------------------
//  CONSTANTS
//----------------------------------------------------------------------------
static const MTypeId TYPE_ID = MTypeId(0x0013b6c0);
static const MString TYPE_NAME = "HideRandomElementsNode";

//----------------------------------------------------------------------------
//  STATIC VARIABLES
//----------------------------------------------------------------------------
MObject HideElementsNode::geometryIn;
MObject HideElementsNode::growIters;
MObject HideElementsNode::hidePercent;
MObject HideElementsNode::geometryOut;

//----------------------------------------------------------------------------
// PUBLIC METHODS
//----------------------------------------------------------------------------
HideElementsNode::HideElementsNode() :
	MPxNode()
{
}


HideElementsNode::~HideElementsNode()
{

}


MStatus HideElementsNode::compute(const MPlug& plug, MDataBlock& data)
{
	TimeProfiler main_profiler = TimeProfiler();
	main_profiler.print_info = MString("compute(): ");
	main_profiler.addTimer();
	MStatus status;
	if (plug == geometryOut)
	{
		int grow_iterations = data.inputValue(growIters).asInt();
		double hide_percentage = data.inputValue(hidePercent).asDouble() * .01;
		MObject geo = data.inputValue(geometryIn).asMesh();
		MDataHandle geometryOutputDataHandle = data.outputValue(geometryOut);

		MItMeshPolygon polygon_itr(geo, &status);
		if (!status)
		{
			MGlobal::displayError("Failed to create polygon iterator: " + status.errorString());
			return(status);
		}

		MUintArray faceIDs = gatherShells(polygon_itr, grow_iterations, hide_percentage);

		if (geo.hasFn(MFn::kMesh))
		{
			MFnMesh meshFn(geo, &status);

			meshFn.setInvisibleFaces(faceIDs);
		}
		geometryOutputDataHandle.setMObject(geo);

		data.setClean(plug);
	}
	main_profiler.stopTimer();
	return (status);
}

//----------------------------------------------------------------------------
// PRIVATE METHODS
//----------------------------------------------------------------------------

MUintArray HideElementsNode::gatherShells(MItMeshPolygon& polygon_itr, const int& grow_iterations, const double& hide_percentage)
{
	TimeProfiler storeFaceIds_profiler = TimeProfiler();
	storeFaceIds_profiler.print_info = MString("storeFaceIds: ");
	TimeProfiler extendToShell_profiler = TimeProfiler();
	extendToShell_profiler.print_info = MString("extendtoShell(): ");
	TimeProfiler removingShells_profiler = TimeProfiler();
	removingShells_profiler.print_info = MString("removeShells: ");
	MString out_str;

	// Store all the element IDs in a MIntArray
	storeFaceIds_profiler.addTimer();
	std::vector<int> faceIDs;
	for (polygon_itr.reset(); !polygon_itr.isDone(); polygon_itr.next())
	{
		faceIDs.push_back(polygon_itr.index());
	}
	storeFaceIds_profiler.stopTimer();

	// pass MItMeshPolygon into the extendToShell() function and get the shell IDs
	MUintArray selected_shells;
	MUintArray shell_ids;

	int count = 0;
	while (faceIDs.size() > 0)
	{
		count++;
		double rand_num = MRandom::Rand_d(count, time(NULL) + count);

		extendToShell_profiler.addTimer();
		shell_ids = extendToShell(polygon_itr, grow_iterations, faceIDs[0]);
		extendToShell_profiler.stopTimer();

		// Remove shell_ids from faceIDs
		removingShells_profiler.addTimer();
		for (const auto& shell_id : shell_ids)
		{
			if (faceIDs.size() > 0)
			{
				faceIDs.erase(std::remove(faceIDs.begin(), faceIDs.end(), shell_id), faceIDs.end());
				if (rand_num > hide_percentage)
				{
					selected_shells.append(shell_id);
				}
			}
			else
			{
				break;
			}
		}
		removingShells_profiler.stopTimer();
	}
	out_str = "Final shell amount: ";
	out_str += count;
	MGlobal::displayInfo(out_str);
	return selected_shells;
}


MUintArray HideElementsNode::extendToShell(MItMeshPolygon& polygon_itr, const int& grow_iterations, const int& start_index)
{
	MUintArray face_ids;
	MStatus status;

	// Full Element ID tree, initialize with start index
	std::vector<std::vector<int>> element_ids;
	element_ids.resize(grow_iterations + 1);
	element_ids[0] = std::vector<int>{ start_index };


	// ID queue to dynamically check what polygons need to be checked
	std::vector<int> id_queue;
	id_queue.push_back(start_index);

	// Iteratively grow the selection based on the amount grow_iterations
	for (int i = 0; i < grow_iterations; i++)
	{
		const int tree_level = i + 1;

		if (id_queue.size() == 0)
		{
			break;
		}

		for (int j = 0; j < id_queue.size(); j++)
		{
			
			MIntArray connected_faces = growSelection(polygon_itr, id_queue[j]);

			for (int k = 0; k < connected_faces.length(); k++)
			{
				// Check if ID is unique before pushing into array
				if (!HideElementsNode::elementExists(element_ids, connected_faces[k]))
				{
					element_ids[tree_level].push_back(connected_faces[k]);
				}
			}
		}
		id_queue = element_ids[tree_level];
	}

	// Fill the MUintArray
	for (int i = 0; i < element_ids.size(); i++)
	{
		for (int j = 0; j < element_ids[i].size(); j++)
		{
			face_ids.append(element_ids[i][j]);
		}
	}

	return (face_ids);
}


MIntArray HideElementsNode::growSelection(MItMeshPolygon& polygon_itr, const int& index)
{
	MIntArray ids;
	int dummy_index;

	polygon_itr.setIndex(index, dummy_index);
	polygon_itr.getConnectedFaces(ids);

	return ids;
}



//----------------------------------------------------------------------------
// STATIC METHODS
//----------------------------------------------------------------------------


bool HideElementsNode::elementExists(const std::vector<std::vector<int>>& array, int item)
{
	std::vector<std::vector<int>>::const_iterator row;

	for (row = array.begin(); row < array.end(); row++)
	{
		if (find(row->begin(), row->end(), item) != row->end())
		{
			return true;
		}
	}
	return false;
}


void* HideElementsNode::Creator()
{
	return(new HideElementsNode());
}


MStatus HideElementsNode::Initialize()
{
	MStatus status;
	MFnTypedAttribute typedAttr;
	MFnNumericAttribute numericAttr;

	MFnMeshData fnMeshData;
	MObject meshDefaultObject = fnMeshData.create(&status);

	// IN ATTRIBUTES
	geometryIn = typedAttr.create("geometryIn", "in", MFnData::kMesh, meshDefaultObject);
	status = typedAttr.setKeyable(true);
	status = typedAttr.setWritable(true);
	status = typedAttr.setReadable(false);

	growIters = numericAttr.create("growIters", "gi", MFnNumericData::kInt, 100);
	status = numericAttr.setKeyable(true);
	status = numericAttr.setWritable(true);
	status = numericAttr.setReadable(false);

	hidePercent = numericAttr.create("hidePercent", "hp", MFnNumericData::kDouble, 50);
	status = numericAttr.setKeyable(true);
	status = numericAttr.setWritable(true);
	status = numericAttr.setReadable(false);

	// OUT ATTRIBUTES
	geometryOut = typedAttr.create("geometryOut", "out", MFnData::kMesh, meshDefaultObject);
	status = typedAttr.setKeyable(true);
	status = typedAttr.setReadable(true);
	status = typedAttr.setWritable(false);

	addAttribute(geometryIn);
	addAttribute(growIters);
	addAttribute(hidePercent);

	addAttribute(geometryOut);

	attributeAffects(geometryIn, geometryOut);
	attributeAffects(growIters, geometryOut);
	attributeAffects(hidePercent, geometryOut);

	return (status);
}


MTypeId HideElementsNode::GetTypeId()
{
	return(TYPE_ID);
}


MString HideElementsNode::GetTypeName()
{
	return(TYPE_NAME);
}
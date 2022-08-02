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
	MStatus status;
	if (plug == geometryOut)
	{
		int grow_iterations = data.inputValue(growIters).asInt();
		double hide_percentage = data.inputValue(hidePercent).asDouble();
		MObject geo = data.inputValue(geometryIn).asMesh();
		MItMeshPolygon polygon_itr(geo, &status);
		if (!status)
		{
			MGlobal::displayError("Failed to create polygon iterator: " + status.errorString());
			return(status);
		}

		gatherShells(polygon_itr, grow_iterations, hide_percentage);

		MDataHandle geometryOutputDataHandle = data.outputValue(geometryOut);
		geometryOutputDataHandle.setMObject(geo);

		data.setClean(plug);
	}

	return (MStatus::kSuccess);
}

//----------------------------------------------------------------------------
// PRIVATE METHODS
//----------------------------------------------------------------------------

MIntArray HideElementsNode::gatherShells(MItMeshPolygon& polygon_itr, const int& grow_iterations, const double& hide_percentage)
{
#ifdef _DEBUG
	TimeProfiler extendToShell_profiler = TimeProfiler();
	extendToShell_profiler.print_info = MString("extendtoShell: ");
	TimeProfiler removingShells_profiler = TimeProfiler();
	removingShells_profiler.print_info = MString("removeShells: ");
#endif
	MString out_str;

	// Store all the element IDs in a MIntArray
	std::vector<int> faceIDs;
	for (polygon_itr.reset(); !polygon_itr.isDone(); polygon_itr.next())
	{
		faceIDs.push_back(polygon_itr.index());
	}
	// pass MItMeshPolygon into the extendToShell() function and get the shell IDs
	MIntArray selected_shells;

	int loop_count = 0;
	MIntArray shell_ids;

	while (faceIDs.size() > 0)
	{
		loop_count++;
		// Safeguard against infinite loops
		if (loop_count > 1000)
		{
			break;
		}
		double rand_num = MRandom::Rand_d(loop_count, time(NULL) + loop_count);
#ifdef _DEBUG
		extendToShell_profiler.addTimer();
#endif 
		shell_ids = extendToShell(polygon_itr, grow_iterations, faceIDs[0]);
#ifdef _DEBUG
		extendToShell_profiler.stopTimer();
#endif 
		// take the returned indices and store them with a random value assigned to that set
#ifdef _DEBUG
		removingShells_profiler.addTimer();
#endif 
		for (unsigned int j = 0; j < shell_ids.length(); j++)
		{
			if (faceIDs.size() > 0)
			{
				faceIDs.erase(std::remove(faceIDs.begin(), faceIDs.end(), shell_ids[j]), faceIDs.end());

				if (rand_num >= hide_percentage)
				{
					selected_shells.append(shell_ids[j]);
				}
			}
			else
			{
				break;
			}
		}
#ifdef _DEBUG
		removingShells_profiler.stopTimer();
#endif
	}
	out_str = "Final shell amount: ";
	out_str += loop_count;
	MGlobal::displayInfo(out_str);
	return selected_shells;
}


MIntArray HideElementsNode::extendToShell(MItMeshPolygon& polygon_itr, const int& grow_iterations, const int& start_index)
{
	MIntArray face_ids;
	MStatus status;
	MString out_str;

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
			
			std::vector<int> connected_faces = growSelection(polygon_itr, element_ids, id_queue[j], tree_level);

			for (int k = 0; k < connected_faces.size(); k++)
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

	// Fill the MIntArray
	for (int i = 0; i < element_ids.size(); i++)
	{
		for (int j = 0; j < element_ids[i].size(); j++)
		{
			face_ids.append(element_ids[i][j]);
		}
	}

	return (face_ids);
}


std::vector<int> HideElementsNode::growSelection(MItMeshPolygon& polygon_itr, std::vector<std::vector<int>>& element_ids, const int& index, const int& depth)
{
	MIntArray ids;
	int dummy_index;
	std::vector<int> connected_ids;

	polygon_itr.setIndex(index, dummy_index);
	polygon_itr.getConnectedFaces(ids);

	for (unsigned int i = 0; i < ids.length(); i++)
	{
		connected_ids.push_back(ids[i]);

	}

	return connected_ids;
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

	growIters = numericAttr.create("growIters", "gi", MFnNumericData::kInt);
	status = numericAttr.setKeyable(true);
	status = numericAttr.setWritable(true);
	status = numericAttr.setReadable(false);

	hidePercent = numericAttr.create("hidePercent", "hp", MFnNumericData::kDouble);
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
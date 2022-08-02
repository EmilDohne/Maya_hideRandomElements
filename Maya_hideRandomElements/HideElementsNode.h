#pragma once

#include <maya/MPxLocatorNode.h>
#include <maya/MIntArray.h>

#include <vector>

class HideElementsNode : public MPxNode
{
public:
	HideElementsNode();
	virtual ~HideElementsNode() override;

	//virtual MStatus doIt(const MArgList& args) override;
	virtual MStatus compute(const MPlug& plug, MDataBlock& data) override;

	//Static methods
	static void* Creator();
	static MStatus Initialize();

	static MTypeId GetTypeId();
	static MString GetTypeName();

private:
	// Input attributes
	static MObject geometryIn;

	// Output attributes
	static MObject geometryOut;


	static bool elementExists(const std::vector<std::vector<int>>& array, int item);

	MIntArray gatherShells(const int& grow_iterations, const double& hide_percentage);
	MIntArray extendToShell(MItMeshPolygon& polygon_itr, const int& grow_iterations, const int& start_index);
	std::vector<int> growSelection(MItMeshPolygon& polygon_itr, std::vector<std::vector<int>>& element_ids, const int& index, const int& depth);

};
#pragma once

#include <maya/MPxLocatorNode.h>
#include <maya/MUIntArray.h>
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
	static MObject growIters;
	static MObject hidePercent;

	// Output attributes
	static MObject geometryOut;

	static bool elementExists(const std::vector<std::vector<int>>& array, int item);

	MUintArray gatherShells(MItMeshPolygon& polygon_itr, const int& grow_iterations, const double& hide_percentage);
	MUintArray extendToShell(MItMeshPolygon& polygon_itr, const int& grow_iterations, const int& start_index);
	MIntArray growSelection(MItMeshPolygon& polygon_itr, const int& index);

};
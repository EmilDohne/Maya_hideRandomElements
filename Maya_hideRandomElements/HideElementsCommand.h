#pragma once

#include <maya/MPxCommand.h>
#include <maya/MIntArray.h>
#include <maya/MSyntax.h>

#include <unordered_map>
#include <vector>

class HideElementsCommand : public MPxCommand
{
public:
	HideElementsCommand();
	virtual ~HideElementsCommand() override;

	virtual MStatus doIt(const MArgList& args) override;

	//Static methods
	static void* Creator();

	static MString CommandName();
	static MSyntax CreateSyntax();

private:

	bool mUndoable;
	bool mEdit;
	bool mQuery;
	bool mIterations;

	static bool elementExists(const std::vector<std::vector<int>>& array, int item);

	MIntArray gatherShells(const int& grow_iterations, const double& hide_percentage);
	MIntArray extendToShell(MItMeshPolygon& polygon_itr, const int& grow_iterations, const int& start_index);
	std::vector<int> growSelection(MItMeshPolygon& polygon_itr, std::vector<std::vector<int>>& element_ids, const int& index, const int& depth);

};
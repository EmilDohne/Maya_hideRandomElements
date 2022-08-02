#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MArgList.h>

#include "HideElementsNode.h"

MStatus initializePlugin(MObject pluginObj)
{
	const char* vendor = "Emil Dohne";
#ifndef _DEBUG 
	const char* version = "0.1.0";
#else
	const char* version = "0.1.0 DEBUG";
#endif
	const char* requiredApiVersion = "Any";

	MStatus status;

	MFnPlugin pluginFn(pluginObj, vendor, version);
	if (!status)
	{
		MGlobal::displayError("Failed to initialize plugin: " + status.errorString());
		return(status);
	}
	
	status = pluginFn.registerNode(HideElementsNode::GetTypeName(),
								   HideElementsNode::GetTypeId(),
								   HideElementsNode::Creator,
								   HideElementsNode::Initialize,
								   HideElementsNode::kDependNode);
	if (!status)
	{
		MGlobal::displayError("Failed to register HideElementsNode: " + status.errorString());
		return(status);
	}

	return(status);
};

MStatus uninitializePlugin(MObject pluginObj)
{
	MStatus status;
	MFnPlugin pluginFn(pluginObj);

	pluginFn.deregisterNode(HideElementsNode::GetTypeId());
	if (!status)
	{
		MGlobal::displayError("Failed to deregister HideElementsNode: " + status.errorString());
		return(status);
	}

	return(status);

}
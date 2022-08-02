#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

#include <maya/MArgList.h>

#include "HideElementsCommand.h"

MStatus initializePlugin(MObject pluginObj)
{
	const char* vendor = "Emil Dohne";
#ifndef _DEBUG 
	const char* version = "1.0.0";
#else
	const char* version = "1.0.0 DEBUG";
#endif
	const char* requiredApiVersion = "Any";

	MStatus status;

	MFnPlugin pluginFn(pluginObj, vendor, version);
	if (!status)
	{
		MGlobal::displayError("Failed to initialize plugin: " + status.errorString());
		return(status);
	}
	
	status = pluginFn.registerCommand(HideElementsCommand::CommandName(), HideElementsCommand::Creator, HideElementsCommand::CreateSyntax);
	if (!status)
	{
		MGlobal::displayError("Failed to register HideElementsCommand: " + status.errorString());
		return(status);
	}

	return(status);
};

MStatus uninitializePlugin(MObject pluginObj)
{
	MStatus status;
	MFnPlugin pluginFn(pluginObj);

	pluginFn.deregisterCommand(HideElementsCommand::CommandName());
	if (!status)
	{
		MGlobal::displayError("Failed to deregister HideElementsCommand: " + status.errorString());
		return(status);
	}

	return(status);

}
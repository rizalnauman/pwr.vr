/*!
\brief A Stream implementation used to access Windows embedded resources.
\file PVRCore/Windows/WindowsResourceStream.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRCore/IO/BufferStream.h"
#include <string>
namespace pvr {
/// <summary>A Stream implementation that is used to access resources built in a MS Windows executable.
/// </summary>
/// <remarks>This Stream abstraction allows the user to easily access the Resources embedded in Microsoft Windows
/// .exe/.dll files created using Windows Resource Files. This is the default way resources are packed in the MS
/// Windows version of the PowerVR Examples.</remarks>
class WindowsResourceStream : public BufferStream
{
public:
	WindowsResourceStream(const std::string& fileName);
	WindowsResourceStream(const std::string& resourceName, const std::string& );
};
}
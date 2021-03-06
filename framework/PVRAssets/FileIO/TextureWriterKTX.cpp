/*!
\brief Implementation of methods of the TextureWriterKTX class.
\file PVRAssets/FileIO/TextureWriterKTX.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRAssets/FileIO/TextureWriterKTX.h"
#include "PVRCore/Texture/TextureDefines.h"

using std::vector;
namespace pvr {
using namespace types;
namespace nativeGles {
namespace ConvertToGles {
//CAUTION: This is a "hidden" dependency on PVRApi. If someone wants to use TextureWriterKTX without PVRApi, he would need to implement
//and link in this function, otherwise there will be linker errors. The implementation can be found (and possibly copied from) PVRApi/TextureUtils.h
bool getOpenGLFormat(PixelFormat pixelFormat, ColorSpace colorSpace, VariableType dataType,
                     uint32& glInternalFormat, uint32& glFormat, uint32& glType, uint32& glTypeSize,
                     bool& isCompressedFormat);
}
}
namespace assets {
namespace assetWriters {

bool TextureWriterKTX::addAssetToWrite(const Texture& asset)
{
	bool result = true;
	if (_assetsToWrite.size() < 1)
	{
		_assetsToWrite.push_back(&asset);
	}
	else
	{
		result = false;
	}

	return result;
}

bool TextureWriterKTX::writeAllAssets()
{
	// Padding data zeroes that we can write later
	const byte* paddingDataZeros[4] = {0, 0, 0, 0};

	// Check the Result
	bool result = true;

	// Create a KTX Texture header
	texture_ktx::FileHeader ktxFileHeader;

	// Set the identifier and endianness
	memcpy(ktxFileHeader.identifier, texture_ktx::c_identifier, sizeof(ktxFileHeader.identifier));
	ktxFileHeader.endianness = texture_ktx::c_endianReference;

	bool isCompressed;
	// Set the pixel format information
	nativeGles::ConvertToGles::getOpenGLFormat(_assetsToWrite[0]->getPixelFormat(), _assetsToWrite[0]->getColorSpace(),
	                                    _assetsToWrite[0]->getChannelType(),
	                                    ktxFileHeader.glInternalFormat, ktxFileHeader.glFormat, ktxFileHeader.glType,
	                                    ktxFileHeader.glTypeSize, isCompressed);

	// Set the dimensions
	ktxFileHeader.pixelWidth = _assetsToWrite[0]->getWidth();
	ktxFileHeader.pixelHeight = _assetsToWrite[0]->getHeight();
	ktxFileHeader.pixelDepth = _assetsToWrite[0]->getDepth();

	// Set the number of surfaces
	ktxFileHeader.numberOfArrayElements = _assetsToWrite[0]->getNumberOfArrayMembers();
	ktxFileHeader.numberOfFaces = _assetsToWrite[0]->getNumberOfFaces();
	ktxFileHeader.numberOfMipmapLevels = _assetsToWrite[0]->getNumberOfMIPLevels();

	// Create the orientation meta data
	string orientationIdentifier(texture_ktx::c_orientationMetaDataKey);
	string orientationString;
	orientationString.append("S=");
	orientationString.push_back(_assetsToWrite[0]->getOrientation(TextureMetaData::AxisAxisX) == TextureMetaData::AxisOrientationLeft
	                            ? 'l' : 'r');
	orientationString.append(",T=");
	orientationString.push_back(_assetsToWrite[0]->getOrientation(TextureMetaData::AxisAxisY) == TextureMetaData::AxisOrientationUp ?
	                            'u' : 'd');
	if (_assetsToWrite[0]->getDepth() > 1)
	{
		orientationString.append(",R=");
		orientationString.push_back(_assetsToWrite[0]->getOrientation(TextureMetaData::AxisAxisY) == TextureMetaData::AxisOrientationUp ?
		                            'u' : 'd');
	}

	// Calculate the amount of orientation meta data (including 2 bytes for NULL characters and 4 bytes for the length of each meta data)
	uint32 orientationMetaDataSize = (uint32)((orientationIdentifier.length() + 1) + (orientationString.length() + 1) + 4);

	// Make sure the length is including padded bytes
	uint32 orientationPaddingSize = 0;
	if (orientationMetaDataSize % 4 != 0)
	{
		orientationPaddingSize = (4 - (orientationMetaDataSize % 4));
	}

	// Set the amount of meta data that's going to be added to this file
	ktxFileHeader.bytesOfKeyValueData = orientationMetaDataSize + orientationPaddingSize;

	// Check the size of data written.
	size_t dataWritten = 0;

	// Write the texture header
	// Write the identifier
	result = _assetStream->write(1, sizeof(ktxFileHeader.identifier), &ktxFileHeader.identifier, dataWritten);
	if (!result || dataWritten != sizeof(ktxFileHeader.identifier)) { return result; }

	// Write the endianness
	result = _assetStream->write(sizeof(ktxFileHeader.endianness), 1, &ktxFileHeader.endianness, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the openGL type
	result = _assetStream->write(sizeof(ktxFileHeader.glType), 1, &ktxFileHeader.glType, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the openGL type size
	result = _assetStream->write(sizeof(ktxFileHeader.glTypeSize), 1, &ktxFileHeader.glTypeSize, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the openGL format
	result = _assetStream->write(sizeof(ktxFileHeader.glFormat), 1, &ktxFileHeader.glFormat, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the openGL internal format
	result = _assetStream->write(sizeof(ktxFileHeader.glInternalFormat), 1, &ktxFileHeader.glInternalFormat, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the openGL base (unsized) internal format
	result = _assetStream->write(sizeof(ktxFileHeader.glBaseInternalFormat), 1, &ktxFileHeader.glBaseInternalFormat, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the width
	result = _assetStream->write(sizeof(ktxFileHeader.pixelWidth), 1, &ktxFileHeader.pixelWidth, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the height
	result = _assetStream->write(sizeof(ktxFileHeader.pixelHeight), 1, &ktxFileHeader.pixelHeight, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the depth
	result = _assetStream->write(sizeof(ktxFileHeader.pixelDepth), 1, &ktxFileHeader.pixelDepth, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the number of array elements
	result = _assetStream->write(sizeof(ktxFileHeader.numberOfArrayElements), 1, &ktxFileHeader.numberOfArrayElements, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the number of faces
	result = _assetStream->write(sizeof(ktxFileHeader.numberOfFaces), 1, &ktxFileHeader.numberOfFaces, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the number of MIP Map levels
	result = _assetStream->write(sizeof(ktxFileHeader.numberOfMipmapLevels), 1, &ktxFileHeader.numberOfMipmapLevels, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the meta data size
	result = _assetStream->write(sizeof(ktxFileHeader.bytesOfKeyValueData), 1, &ktxFileHeader.bytesOfKeyValueData, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the size of the orientation data
	result = _assetStream->write(sizeof(orientationMetaDataSize), 1, &orientationMetaDataSize, dataWritten);
	if (!result || dataWritten != 1) { return result; }

	// Write the orientation data key
	result = _assetStream->write(1, orientationIdentifier.length() + 1, orientationIdentifier.c_str(), dataWritten);
	if (!result || dataWritten != orientationIdentifier.length() + 1) { return result; }

	// Write the orientation data values
	result = _assetStream->write(1, orientationString.length() + 1, orientationString.c_str(), dataWritten);
	if (!result || dataWritten != orientationString.length() + 1) { return result; }

	// Write in any padding data, use zeros for safety.
	result = _assetStream->write(1, orientationPaddingSize, paddingDataZeros, dataWritten);
	if (!result || dataWritten != orientationPaddingSize) { return result; }

	// Write the texture data
	for (uint32 mipMapLevel = 0; mipMapLevel < ktxFileHeader.numberOfMipmapLevels; ++mipMapLevel)
	{
		// Calculate the MIP map size - regular cube maps are a slight exception
		uint32 mipMapSize = 0;
		if (_assetsToWrite[0]->getNumberOfFaces() == 6 && _assetsToWrite[0]->getNumberOfArrayMembers() == 1)
		{
			mipMapSize = _assetsToWrite[0]->getDataSize(mipMapLevel, false, false);
		}
		else
		{
			mipMapSize = _assetsToWrite[0]->getDataSize(mipMapLevel);
		}

		// Write the stored size of the MIP Map.
		result = _assetStream->write(sizeof(mipMapSize), 1, &mipMapSize, dataWritten);
		if (!result || dataWritten != 1) { return result; }

		// Work out the Cube Map padding.
		uint32 cubePadding = 0;
		if (_assetsToWrite[0]->getDataSize(mipMapLevel, false, false) % 4)
		{
			cubePadding = 4 - (_assetsToWrite[0]->getDataSize(mipMapLevel, false, false) % 4);
		}

		// Compressed images are written without scan line padding, because there aren't necessarily any scan lines.
		if (_assetsToWrite[0]->getPixelFormat().getPart().High == 0 &&
		    _assetsToWrite[0]->getPixelFormat().getPixelTypeId() != (uint64)CompressedPixelFormat::SharedExponentR9G9B9E5)
		{
			for (uint32 iSurface = 0; iSurface < _assetsToWrite[0]->getNumberOfArrayMembers(); ++iSurface)
			{
				for (uint32 iFace = 0; iFace < _assetsToWrite[0]->getNumberOfFaces(); ++iFace)
				{
					// Write in the texture data.
					result = _assetStream->write(_assetsToWrite[0]->getDataSize(mipMapLevel, false, false), 1,
					                              _assetsToWrite[0]->getDataPointer(mipMapLevel, iSurface, iFace), dataWritten);
					if (!result || dataWritten != 1) { return result; }

					// Advance past the cube face padding
					if (cubePadding && _assetsToWrite[0]->getNumberOfFaces() == 6 && _assetsToWrite[0]->getNumberOfArrayMembers() == 1)
					{
						result = _assetStream->write(1, cubePadding, paddingDataZeros, dataWritten);
						if (!result || dataWritten != cubePadding) { return result; }
					}
				}
			}
		}
		// Uncompressed images have scan line padding.
		else
		{
			for (uint32 iSurface = 0; iSurface < _assetsToWrite[0]->getNumberOfArrayMembers(); ++iSurface)
			{
				for (uint32 iFace = 0; iFace < _assetsToWrite[0]->getNumberOfFaces(); ++iFace)
				{
					for (uint32 texDepth = 0; texDepth < _assetsToWrite[0]->getDepth(); ++texDepth)
					{
						for (uint32 texHeight = 0; texHeight < _assetsToWrite[0]->getHeight(); ++texHeight)
						{
							// Calculate the data offset for the relevant scan line
							uint64 scanLineOffset = (textureOffset3D(0, texHeight, texDepth, _assetsToWrite[0]->getWidth(),
							                         _assetsToWrite[0]->getHeight()) * (_assetsToWrite[0]->getBitsPerPixel() / 8));
							// Write in the texture data for the current scan line.
							result = _assetStream->write((_assetsToWrite[0]->getBitsPerPixel() / 8) *
							                              _assetsToWrite[0]->getWidth(mipMapLevel), 1,
							                              _assetsToWrite[0]->getDataPointer(mipMapLevel, iSurface, iFace) +
							                              scanLineOffset, dataWritten);
							if (!result || dataWritten != 1) { return result; }

							// Work out the amount of scan line padding.
							uint32 scanLinePadding = (static_cast<uint32>(-1) * ((_assetsToWrite[0]->getBitsPerPixel() / 8) *
							                          _assetsToWrite[0]->getWidth(mipMapLevel))) % 4;

							// Advance past the scan line padding
							if (scanLinePadding)
							{
								result = _assetStream->write(1, scanLinePadding, paddingDataZeros, dataWritten);
								if (!result || dataWritten != scanLinePadding) { return result; }
							}
						}
					}

					// Advance past the cube face padding
					if (cubePadding && _assetsToWrite[0]->getNumberOfFaces() == 6 && _assetsToWrite[0]->getNumberOfArrayMembers() == 1)
					{
						result = _assetStream->write(1, cubePadding, paddingDataZeros, dataWritten);
						if (!result || dataWritten != cubePadding) { return result; }
					}
				}
			}
		}

		// Calculate the amount MIP Map padding.
		uint32 mipMapPadding = (3 - ((mipMapSize + 3) % 4));

		// Write MIP Map padding
		if (mipMapPadding)
		{
			result = _assetStream->write(1, mipMapPadding, paddingDataZeros, dataWritten);
			if (!result || dataWritten != mipMapPadding) { return result; }
		}
	}

	// Return
	return result;
}

uint32 TextureWriterKTX::assetsAddedSoFar()
{
	return (uint32)_assetsToWrite.size();
}

bool TextureWriterKTX::supportsMultipleAssets()
{
	return false;
}

bool TextureWriterKTX::canWriteAsset(const Texture& asset)
{
	// Create a KTX Texture header to read the format into.
	texture_ktx::FileHeader ktxFileHeader; bool isCompressed;

	// Check if the pixel format is supported
	return nativeGles::ConvertToGles::getOpenGLFormat(asset.getPixelFormat(), asset.getColorSpace(), asset.getChannelType(), ktxFileHeader.glInternalFormat,
	       ktxFileHeader.glFormat, ktxFileHeader.glType, ktxFileHeader.glTypeSize, isCompressed);
}

vector<string> TextureWriterKTX::getSupportedFileExtensions()
{
	vector<string> extensions;
	extensions.push_back("ktx");
	return vector<string>(extensions);
}

string TextureWriterKTX::getWriterName()
{
	return "PowerVR Khronos Texture Writer";
}

string TextureWriterKTX::getWriterVersion()
{
	return "1.0.0";
}
}
}
}
//!\endcond
/*!
\brief Contains the OpenGL ES 2/3 implementation of the all-important pvr::api::GraphicsPipeline object.
\file PVRApi/OGLES/GraphicsPipelineGles.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRApi/ApiIncludes.h"
#include "PVRApi/OGLES/StateContainerGles.h"
#include "PVRNativeApi/OGLES/OpenGLESBindings.h"
#include "PVRNativeApi/OGLES/NativeObjectsGles.h"
#include "PVRNativeApi/OGLES/ShaderUtilsGles.h"
#include "PVRApi/OGLES/ShaderGles.h"
#include "PVRApi/OGLES/ContextGles.h"
#include "PVRApi/OGLES/GraphicsPipelineGles.h"
namespace pvr {
namespace api {
namespace pipelineCreation {
void createStateObjects(const DepthStencilStateCreateParam& thisobject, gles::GraphicsStateContainer& storage, const  DepthStencilStateCreateParam* parent_param);
void createStateObjects(const ColorBlendStateCreateParam& thisobject, gles::GraphicsStateContainer& storage, const  ColorBlendStateCreateParam* parent_param);
void createStateObjects(const ViewportStateCreateParam& thisobject, gles::GraphicsStateContainer& storage, const  ViewportStateCreateParam* parent_param);
void createStateObjects(const RasterStateCreateParam& thisobject, gles::GraphicsStateContainer& storage, const  RasterStateCreateParam* parent_param);
void createStateObjects(const VertexInputCreateParam& thisobject, gles::GraphicsStateContainer& storage, const  VertexInputCreateParam* parent_param);
void createStateObjects(const InputAssemblerStateCreateParam& thisobject, gles::GraphicsStateContainer& storage, const  InputAssemblerStateCreateParam* parent_param);
void createStateObjects(const VertexShaderStageCreateParam& thisobject, gles::GraphicsStateContainer& storage, const  VertexShaderStageCreateParam* parent_param);
void createStateObjects(const FragmentShaderStageCreateParam& thisobject, gles::GraphicsStateContainer& storage, const  FragmentShaderStageCreateParam* parent_param);
void createStateObjects(const GeometryShaderStageCreateParam& thisobject, gles::GraphicsStateContainer& storage, const  GeometryShaderStageCreateParam* parent_param);
void createStateObjects(const TesselationStageCreateParam& thisobject, gles::GraphicsStateContainer& storage, const  TesselationStageCreateParam* parent_param);
}
namespace gles {
class PushPipeline;
class PopPipeline;
class ResetPipeline;
template<typename> class PackagedBindable;
template<typename, typename> class PackagedBindableWithParam;
class ParentableGraphicsPipeline_;

//////IMPLEMENTATION INFO/////
/*The desired class hierarchy was
---- OUTSIDE INTERFACE ----
* ParentableGraphicsPipeline(PGP)     : GraphicsPipeline(GP)
-- Inside implementation --
* ParentableGraphicsPipelineGles(PGPGles) : GraphicsPipelineGles(GPGles)
* GraphicsPipelineGles(GPGles)        : GraphicsPipeline(GP)
---------------------------
This would cause a diamond inheritance, with PGPGles inheriting twice from GP, once through PGP and once through GPGles.
To avoid this issue while maintaining the outside interface, the pImpl idiom is being used instead of the inheritance
chains commonly used for all other PVRApi objects. The same idiom (for the same reasons) is found in the CommandBuffer.
*/////////////////////////////

namespace {
struct PipelineStatePointerLess
{
	inline bool operator()(const impl::PipelineState* lhs, const impl::PipelineState* rhs) const
	{
		return static_cast<int32>(lhs->getStateType()) < static_cast<int32>(rhs->getStateType());
	}
};
struct PipelineStatePointerGreater
{
	inline bool operator()(const  impl::PipelineState* lhs, const  impl::PipelineState* rhs) const
	{
		return static_cast<int32>(lhs->getStateType()) > static_cast<int32>(rhs->getStateType());
	}
};

inline int32 getUniformLocation_(const char8* uniform, GLuint prog)
{
	int32 ret = gl::GetUniformLocation(prog, uniform);
	if (ret == -1)
	{
		Log(Log.Debug, "GraphicsPipeline::getUniformLocation [%s] for program [%d]  returned -1: Uniform was not active", uniform, prog);
	}
	return ret;
}
inline int32 getAttributeLocation_(const char8* attribute, GLuint prog)
{
	int32 ret = gl::GetAttribLocation(prog, attribute);
	if (ret == -1)
	{
		Log(Log.Debug, "GraphicsPipeline::getAttributeLocation [%s] for program [%d]  returned -1: Attribute was not active", attribute, prog);
	}
	return ret;
}
}

static GraphicsShaderProgramState dummy_state;

const GraphicsShaderProgramState& GraphicsPipelineImplGles::getShaderProgram()const
{
	if (!_states.numStates() || _states.states[0]->getStateType() !=
	    impl::GraphicsStateType::ShaderProgram)
	{
		if (_parent)
		{
			return static_cast<const GraphicsPipelineImplGles&>(_parent->getImpl()).getShaderProgram();
		}
		else
		{
			return dummy_state;
		}
	}
	return *(static_cast<gles::GraphicsShaderProgramState*>(_states.states[0]));
}
GraphicsShaderProgramState& GraphicsPipelineImplGles::getShaderProgram()
{
	if (!_states.numStates() || _states.states[0]->getStateType() !=
	    impl::GraphicsStateType::ShaderProgram)
	{
		if (_parent)
		{
			return static_cast<GraphicsPipelineImplGles&>(_parent->getImpl()).getShaderProgram();
		}
		else
		{
			return dummy_state;
		}
	}
	return *(static_cast<gles::GraphicsShaderProgramState*>(_states.states[0]));
}


const GraphicsPipelineCreateParam& GraphicsPipelineImplGles::getCreateParam()const
{
	return _createParam;
}

int32 GraphicsPipelineImplGles::getAttributeLocation(const char8* attribute)const
{
	return getAttributeLocation_(attribute, native_cast(getShaderProgram()));
}

void GraphicsPipelineImplGles::getAttributeLocation(const char8** attributes, uint32 numAttributes, int32* outLocation)const
{
	GLint prog = native_cast(getShaderProgram());
	for (uint32 i = 0; i < numAttributes; ++i)
	{
		outLocation[i] = getAttributeLocation_(attributes[i], prog);
	}
}

int32 GraphicsPipelineImplGles::getUniformLocation(const char8* uniform)const
{
	return getUniformLocation_(uniform, native_cast(getShaderProgram()));
}

void GraphicsPipelineImplGles::getUniformLocation(const char8** uniforms, uint32 numUniforms, int32* outLocation)const
{
	GLuint prog = native_cast(getShaderProgram());
	for (uint32 i = 0; i < numUniforms; ++i)
	{
		outLocation[i] = getUniformLocation_(uniforms[i], prog);
	}
}

pvr::uint8 GraphicsPipelineImplGles::getNumAttributes(pvr::uint16 bindingId)const
{
	return _states.getNumAttributes(bindingId);
}

const VertexInputBindingInfo* GraphicsPipelineImplGles::getInputBindingInfo(pvr::uint16 bindingId)const
{
	return _states.getInputBindingInfo(bindingId);
}

const VertexAttributeInfoWithBinding* GraphicsPipelineImplGles::getAttributesInfo(pvr::uint16 bindId)const
{
	return _states.getAttributesInfo(bindId);
}

const pvr::api::PipelineLayout& GraphicsPipelineImplGles::getPipelineLayout() const
{
	// return the pipeline layout / else return the valid parent's pipeline layout else NULL object
	if (_states.pipelineLayout.isNull() && _parent)
	{
		return _parent->getPipelineLayout();
	}
	assertion(!_states.pipelineLayout.isNull(), "invalid pipeline layout");
	return _states.pipelineLayout;
}

void GraphicsPipelineImplGles::setFromParent() { _states.setAll(*_context); }

void GraphicsPipelineImplGles::setAll()
{
	debugLogApiError("GraphicsPipeline::setAll entry");
	if (_parent)
	{
		static_cast<ParentableGraphicsPipelineImplGles&>(_parent->getImpl()).setAll();
	}
	setFromParent();
	debugLogApiError("GraphicsPipeline::setAll exit");
}


void GraphicsPipelineImplGles::destroy()
{
	_states.vertexShader.reset();
	_states.fragmentShader.reset();
	_states.geometryShader.reset();
	_states.tessControlShader.reset();
	_states.tessEvalShader.reset();
	_states.vertexInputBindings.clear();
	for (gles::GraphicsStateContainer::StateContainerIter it = _states.states.begin();
	     it != _states.states.end(); ++it)
	{
		delete *it;
	}
	_states.states.clear();
	_states.clear();
	_parent = 0;
}

bool GraphicsPipelineImplGles::initBase(const GraphicsPipelineCreateParam& desc, gles::GraphicsStateContainer& states, ParentableGraphicsPipeline& parent)
{
	states.pipelineLayout = desc.pipelineLayout;
	if (!states.pipelineLayout.isValid() && (parent.isValid() && !parent->getPipelineLayout().isValid()))
	{
		pvr::Log(pvr::Logger::Error, "Invalid Pipeline Layout");
		return false;
	}
	if (desc.colorBlend.getAttachmentStatesCount() == 0 || (parent.isValid() && parent->getCreateParam().colorBlend.getAttachmentStatesCount() == 0))
	{
		pvr::Log("Pipeline must have atleast one color attachment state");
		return false;
	}
	pipelineCreation::createStateObjects(desc.colorBlend, states, (_parent ? &_parent->getCreateParam().colorBlend : NULL));
	pipelineCreation::createStateObjects(desc.depthStencil, states, (_parent ? &_parent->getCreateParam().depthStencil : NULL));
	pipelineCreation::createStateObjects(desc.fragmentShader, states, (_parent ? &_parent->getCreateParam().fragmentShader : NULL));
	pipelineCreation::createStateObjects(desc.vertexShader, states, (_parent ? &_parent->getCreateParam().vertexShader : NULL));
	pipelineCreation::createStateObjects(desc.inputAssembler, states, (_parent ? &_parent->getCreateParam().inputAssembler : NULL));
	pipelineCreation::createStateObjects(desc.rasterizer, states, (_parent ? &_parent->getCreateParam().rasterizer : NULL));
	pipelineCreation::createStateObjects(desc.vertexInput, states, (_parent ? &_parent->getCreateParam().vertexInput : NULL));
	pipelineCreation::createStateObjects(desc.viewport, states, (_parent ? &_parent->getCreateParam().viewport : NULL));
	pipelineCreation::createStateObjects(desc.geometryShader, states, (_parent ? &_parent->getCreateParam().geometryShader : NULL));
	pipelineCreation::createStateObjects(desc.tesselationStates, states, (_parent ? &_parent->getCreateParam().tesselationStates : NULL));

	if (!states.hasVertexShader() || !states.hasFragmentShader())
	{
		if ((!parent.isNull()) && (!static_cast<const GraphicsPipelineImplGles&>(_parent->getImpl())._states.hasVertexShader() ||
		                           !static_cast<const GraphicsPipelineImplGles&>(_parent->getImpl())._states.hasFragmentShader()))
		{
			pvr::Log(Log.Error, "GraphicsPipeline:: Shaders were invalid, and parent pipeline did not contain shaders.");
			return false;
		}
	}

	Result retval = Result::Success;
	if (states.hasVertexShader() && states.hasFragmentShader())
	{
		retval = createProgram();
	}
	else if (parent.isNull())
	{
		pvr::Log(Log.Error, "GraphicsPipeline:: Shaders were invalid");
		retval = Result::InvalidData;
	}
	if (retval != pvr::Result::Success)
	{
		pvr::Log(Log.Error, "GraphicsPipeline:: Program creation unsuccessful.");
		return false;
	}
	//Invariant: no duplicates created.
	gles::GraphicsStateContainer& containerGles = states;
	std::sort(containerGles.states.begin(), containerGles.states.end(), PipelineStatePointerLess());

	PipelineStatePointerLess  compLess;
	PipelineStatePointerGreater compGreater;
	uint32 counterChild = 0, counterParent = 0;

	while (_parent != NULL && containerGles.states.size() > counterChild
	       && static_cast<const GraphicsPipelineImplGles&>(_parent->getImpl())._states.states.size() > counterParent)
	{
		if (compLess(containerGles.states[counterChild],
		             static_cast<const GraphicsPipelineImplGles&>(_parent->getImpl())._states.states[counterParent]))
		{
			counterChild++;
		}
		else if (compGreater(containerGles.states[counterChild],
		                     static_cast<const GraphicsPipelineImplGles&>(_parent->getImpl())._states.states[counterParent]))
		{
			counterParent++;
		}
		else
		{
			containerGles.states[counterChild]->_parent =
			  static_cast<const ParentableGraphicsPipelineImplGles&>(_parent->getImpl())._states.states[counterParent];
			counterChild++;
			counterParent++;
		}
	}

	const pipelineCreation::OGLES2TextureUnitBindings* texUnitBinding = NULL;

	if (desc.es2TextureBindings.getBindingCount())
	{
		texUnitBinding = &desc.es2TextureBindings;
	}
	else if (_parent && _parent->getCreateParam().es2TextureBindings.getBindingCount())
	{
		texUnitBinding = &_parent->getCreateParam().es2TextureBindings;
	}
	if (texUnitBinding)
	{
		platform::ContextGles::RenderStatesTracker& stateTracker = static_cast<platform::ContextGles&>(*_context).getCurrentRenderStates();
		GLuint last_prog = stateTracker.lastBoundProgram;
		GLuint prog = native_cast(getShaderProgram());
		for (uint32 i = 0; i < texUnitBinding->getBindingCount(); ++i)
		{
			int32 uniformLoc = getUniformLocation(texUnitBinding->getTextureUnitName(i));
			if (uniformLoc >= 0)
			{
				if (stateTracker.lastBoundProgram != prog)
				{
					gl::UseProgram(prog);
					stateTracker.lastBoundProgram = prog;
				}
				gl::Uniform1i(uniformLoc, i);
			}
		}
		if (stateTracker.lastBoundProgram != last_prog)
		{
			gl::UseProgram(last_prog);
			stateTracker.lastBoundProgram = last_prog;
		}
	}
	_initialized = true;

	return retval == Result::Success;
}

bool ParentableGraphicsPipelineImplGles::init(const GraphicsPipelineCreateParam& desc, ParentableGraphicsPipeline& owner)
{
	if (_initialized) { pvr::Log(pvr::Logger::Debug, "Pipeline is already initialized"); return true; }
	_parent = NULL;
	_owner = owner.get();
	_createParam = desc;

	return GraphicsPipelineImplGles::initBase(desc, _states, owner);
}

bool GraphicsPipelineImplGles::init(const GraphicsPipelineCreateParam& desc, ParentableGraphicsPipeline& parent,
                                    GraphicsPipeline& owner)
{
	if (_initialized) { pvr::Log(pvr::Logger::Debug, "Pipeline is already initialized"); return true; }
	_parent = parent.get();
	_owner = owner.get();
	_createParam = desc;

	return GraphicsPipelineImplGles::initBase(desc, _states, parent);
}

Result GraphicsPipelineImplGles::createProgram()
{
	gles::GraphicsShaderProgramState* program = new gles::GraphicsShaderProgramState();
	gles::GraphicsStateContainer& containerGles = _states;

	std::vector<native::HShader_> shaders;
	shaders.push_back(static_cast<ShaderGles_&>(*containerGles.vertexShader));
	shaders.push_back(static_cast<ShaderGles_&>(*containerGles.fragmentShader));
	if (containerGles.geometryShader.isValid())
	{
		shaders.push_back(native_cast(*containerGles.geometryShader));
	}
	if (containerGles.tessControlShader.isValid())
	{
		shaders.push_back(native_cast(*containerGles.tessControlShader));
	}
	if (containerGles.tessEvalShader.isValid())
	{
		shaders.push_back(native_cast(*containerGles.tessEvalShader));
	}

	std::vector<const char*> attribNames;
	std::vector<uint16> attribIndex;

	// retrive the attribute names and index
	for (auto it = containerGles.vertexAttributes.begin(); it != containerGles.vertexAttributes.end(); ++it)
	{
		attribNames.push_back(it->attribName.c_str());
		attribIndex.push_back(it->index);
	}
	std::vector<int> vec;
	std::string errorStr;
	const char** attribs = (attribNames.size() ? &attribNames[0] : NULL);
	if (!nativeGles::createShaderProgram(&shaders[0], (uint32)shaders.size(), attribs, attribIndex.data(),
	                                     (uint32)attribIndex.size(), native_cast(*program), &errorStr,
	                                     &_context->getApiCapabilities()))
	{
		Log(Log.Critical, "Linking failed. Shader infolog: %s", errorStr.c_str());
		return Result::InvalidData;
	}
	containerGles.states.push_back(program);
	return Result::Success;
}// namespace gles
}
}
}

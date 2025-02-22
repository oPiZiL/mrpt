/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include "opengl-precomp.h"	 // Precompiled header
//
#include <mrpt/core/get_env.h>
#include <mrpt/core/lock_helper.h>
#include <mrpt/opengl/CRenderizableShaderTexturedTriangles.h>
#include <mrpt/opengl/TLightParameters.h>
#include <mrpt/opengl/opengl_api.h>
#include <mrpt/serialization/CArchive.h>
#include <mrpt/system/CTimeLogger.h>

#include <iostream>
#include <memory>  // std::align
#include <set>
#include <thread>

using namespace mrpt;
using namespace mrpt::opengl;
using namespace mrpt::poses;
using namespace mrpt::math;
using namespace std;
using mrpt::img::CImage;

IMPLEMENTS_VIRTUAL_SERIALIZABLE(
	CRenderizableShaderTexturedTriangles, CRenderizable, mrpt::opengl)

const bool MRPT_OPENGL_VERBOSE =
	mrpt::get_env<bool>("MRPT_OPENGL_VERBOSE", false);

// Whether to profile memory allocations:
//#define TEXTUREOBJ_PROFILE_MEM_ALLOC

// Whether to use a memory pool for the texture buffer:
#define TEXTUREOBJ_USE_MEMPOOL

void CRenderizableShaderTexturedTriangles::renderUpdateBuffers() const
{
#if MRPT_HAS_OPENGL_GLUT
	// Generate vertices & colors into m_triangles
	const_cast<CRenderizableShaderTexturedTriangles&>(*this)
		.onUpdateBuffers_TexturedTriangles();

	const auto n = m_triangles.size();

	// Define OpenGL buffers:
	m_vertexBuffer.createOnce();
	m_vertexBuffer.bind();
	m_vertexBuffer.allocate(m_triangles.data(), sizeof(m_triangles[0]) * n);

	// VAO: required to use glEnableVertexAttribArray()
	m_vao.createOnce();

#endif
}

void CRenderizableShaderTexturedTriangles::render(const RenderContext& rc) const
{
#if MRPT_HAS_OPENGL_GLUT

	// This will load and/or select our texture, only once:
	initializeTextures();
	ASSERT_(m_glTexture.has_value());

	// Set the texture uniform:
	{
		const Program& s = *rc.shader;
		// bound to GL_TEXTURE0 + "i":
		glUniform1i(s.uniformId("textureSampler"), m_glTexture->unit);
	}

	// Enable/disable lights:
	if (rc.shader->hasUniform("enableLight"))
	{
		const Program& s = *rc.shader;
		GLint enabled = m_enableLight ? 1 : 0;
		glUniform1i(s.uniformId("enableLight"), enabled);
		CHECK_OPENGL_ERROR();
	}

	if (m_enableLight && rc.lights && rc.shader->hasUniform("light_diffuse") &&
		rc.shader->hasUniform("light_ambient") &&
		rc.shader->hasUniform("light_direction"))
	{
		const Program& s = *rc.shader;
		glUniform4fv(s.uniformId("light_diffuse"), 1, &rc.lights->diffuse.R);
		glUniform4fv(s.uniformId("light_ambient"), 1, &rc.lights->ambient.R);
		// glUniform4fv(s.uniformId("light_specular"), 1,
		// &rc.lights->specular.R);
		glUniform3fv(
			s.uniformId("light_direction"), 1, &rc.lights->direction.x);
		CHECK_OPENGL_ERROR();
	}

	// Set up the vertex array:
	std::optional<GLuint> attr_position;
	if (rc.shader->hasAttribute("position"))
	{
		attr_position = rc.shader->attributeId("position");
		m_vao.bind();
		glEnableVertexAttribArray(*attr_position);
		m_vertexBuffer.bind();
		glVertexAttribPointer(
			*attr_position, /* attribute */
			3, /* size */
			GL_FLOAT, /* type */
			GL_FALSE, /* normalized? */
			sizeof(TTriangle::Vertex), /* stride */
			BUFFER_OFFSET(offsetof(TTriangle::Vertex, xyzrgba.pt.x)));
		CHECK_OPENGL_ERROR();
	}

	// Set up the normals array:
	std::optional<GLuint> attr_normals;
	if (rc.shader->hasAttribute("vertexNormal"))
	{
		attr_normals = rc.shader->attributeId("vertexNormal");
		glEnableVertexAttribArray(*attr_normals);
		m_vertexBuffer.bind();
		glVertexAttribPointer(
			*attr_normals, /* attribute */
			3, /* size */
			GL_FLOAT, /* type */
			GL_FALSE, /* normalized? */
			sizeof(TTriangle::Vertex), /* stride */
			BUFFER_OFFSET(offsetof(TTriangle::Vertex, normal.x)));
		CHECK_OPENGL_ERROR();
	}

	// Set up the UV array:
	std::optional<GLuint> attr_uv;
	if (rc.shader->hasAttribute("vertexUV"))
	{
		attr_uv = rc.shader->attributeId("vertexUV");
		glEnableVertexAttribArray(*attr_uv);
		m_vertexBuffer.bind();
		glVertexAttribPointer(
			*attr_uv, /* attribute */
			2, /* size */
			GL_FLOAT, /* type */
			GL_FALSE, /* normalized? */
			sizeof(TTriangle::Vertex), /* stride */
			BUFFER_OFFSET(offsetof(TTriangle::Vertex, uv.x)));
		CHECK_OPENGL_ERROR();
	}

	if (m_cullface == TCullFace::NONE) { glDisable(GL_CULL_FACE); }
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(m_cullface == TCullFace::FRONT ? GL_FRONT : GL_BACK);
		CHECK_OPENGL_ERROR();
	}

	// Draw:
	glDrawArrays(GL_TRIANGLES, 0, 3 * m_triangles.size());
	CHECK_OPENGL_ERROR();

	glDisable(GL_CULL_FACE);
	if (attr_position) glDisableVertexAttribArray(*attr_position);
	if (attr_uv) glDisableVertexAttribArray(*attr_uv);
	if (attr_normals) glDisableVertexAttribArray(*attr_normals);

#endif
}

// Data types for memory pooling CRenderizableShaderTexturedTriangles:
#ifdef TEXTUREOBJ_USE_MEMPOOL

#include <mrpt/system/CGenericMemoryPool.h>

struct CRenderizableShaderTexturedTriangles_MemPoolParams
{
	/** size of the vector<unsigned char> */
	size_t len = 0;

	inline bool isSuitable(
		const CRenderizableShaderTexturedTriangles_MemPoolParams& req) const
	{
		return len == req.len;
	}
};
struct CRenderizableShaderTexturedTriangles_MemPoolData
{
	vector<unsigned char> data;
};

using TMyMemPool = mrpt::system::CGenericMemoryPool<
	CRenderizableShaderTexturedTriangles_MemPoolParams,
	CRenderizableShaderTexturedTriangles_MemPoolData>;
#endif

void CRenderizableShaderTexturedTriangles::assignImage(
	const CImage& img, const CImage& imgAlpha)
{
	MRPT_START

	CRenderizable::notifyChange();

	unloadTexture();

	// Make a copy:
	m_textureImage = img;
	m_textureImageAlpha = imgAlpha;
	m_textureImageAssigned = true;

	m_enableTransparency = true;

	MRPT_END
}

void CRenderizableShaderTexturedTriangles::assignImage(const CImage& img)
{
	MRPT_START

	CRenderizable::notifyChange();

	unloadTexture();

	// Make a copy:
	m_textureImage = img;
	m_textureImageAssigned = true;

	m_enableTransparency = false;

	MRPT_END
}

void CRenderizableShaderTexturedTriangles::assignImage(
	CImage&& img, CImage&& imgAlpha)
{
	MRPT_START

	CRenderizable::notifyChange();

	unloadTexture();

	m_textureImage = std::move(img);
	m_textureImageAlpha = std::move(imgAlpha);
	m_textureImageAssigned = true;

	m_enableTransparency = true;

	MRPT_END
}

void CRenderizableShaderTexturedTriangles::assignImage(CImage&& img)
{
	MRPT_START

	CRenderizable::notifyChange();

	unloadTexture();

	m_textureImage = std::move(img);
	m_textureImageAssigned = true;

	m_enableTransparency = false;

	MRPT_END
}

// Auxiliary function for loadTextureInOpenGL(): reserve memory and return
// 16byte aligned starting point within it:
static unsigned char* reserveDataBuffer(
	const size_t len, vector<unsigned char>& data)
{
#ifdef TEXTUREOBJ_USE_MEMPOOL
	TMyMemPool* pool = TMyMemPool::getInstance();
	if (pool)
	{
		CRenderizableShaderTexturedTriangles_MemPoolParams mem_params;
		mem_params.len = len;

		CRenderizableShaderTexturedTriangles_MemPoolData* mem_block =
			pool->request_memory(mem_params);
		if (mem_block)
		{
			// Recover the memory block via a swap:
			data.swap(mem_block->data);
			delete mem_block;
		}
	}
#endif
	data.resize(len);
	void* ptr = &data[0];
	size_t space = len;
	return reinterpret_cast<unsigned char*>(
		std::align(16, 1 /*dummy size*/, ptr, space));
}

void CRenderizableShaderTexturedTriangles::initializeTextures() const
{
#if MRPT_HAS_OPENGL_GLUT
	unsigned char* dataAligned = nullptr;
	vector<unsigned char> data;

#ifdef TEXTUREOBJ_PROFILE_MEM_ALLOC
	static mrpt::system::CTimeLogger tim;
#endif

	// Note: if we are rendering and the user assigned us no texture image,
	// let's create a dummy one with the uniform CRenderizable's color:
	if (!textureImageHasBeenAssigned() || m_textureImage.isEmpty())
	{
		mrpt::img::CImage im_rgb(4, 4, mrpt::img::CH_RGB),
			im_a(4, 4, mrpt::img::CH_GRAY);
		im_rgb.filledRectangle(0, 0, 3, 3, m_color);
		im_a.filledRectangle(
			0, 0, 3, 3,
			mrpt::img::TColor(m_color.A, m_color.A, m_color.A, m_color.A));
		const_cast<CRenderizableShaderTexturedTriangles*>(this)->assignImage(
			std::move(im_rgb), std::move(im_a));
	}

	try
	{
		if (m_glTexture.has_value())
		{
			// activate the texture unit first before binding texture
			glActiveTexture(GL_TEXTURE0 + m_glTexture->unit);
			glBindTexture(GL_TEXTURE_2D, m_glTexture->name);
			CHECK_OPENGL_ERROR();
			return;
		}

		// Reserve the new one --------------------------
		m_textureImage.forceLoad();	 // just in case they are lazy-load imgs
		m_textureImageAlpha.forceLoad();

		ASSERT_(m_textureImage.getPixelDepth() == mrpt::img::PixelDepth::D8U);

		// allocate texture names:
		m_glTexture = getNewTextureNumber();

		// activate the texture unit first before binding texture
		glActiveTexture(GL_TEXTURE0 + m_glTexture->unit);
		// select our current texture
		glBindTexture(GL_TEXTURE_2D, m_glTexture->name);
		CHECK_OPENGL_ERROR();

		// when texture area is small, linear interpolation. Default is
		// GL_LINEAR_MIPMAP_NEAREST but we
		// are not building mipmaps.
		//  See also:
		//  http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=133116&page=1
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		CHECK_OPENGL_ERROR();

		// when texture area is large, NEAREST: this is mainly thinking of
		// rendering
		//  occupancy grid maps, such as we want those "big pixels" to be
		//  clearly visible ;-)
		glTexParameterf(
			GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			m_textureInterpolate ? GL_LINEAR : GL_NEAREST);
		CHECK_OPENGL_ERROR();

		// if wrap is true, the texture wraps over at the edges (repeat)
		//       ... false, the texture ends at the edges (clamp)
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		CHECK_OPENGL_ERROR();

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		CHECK_OPENGL_ERROR();

		// Assure that the images do not overpass the maximum dimensions allowed
		// by OpenGL:
		// ------------------------------------------------------------------------------------
		GLint texSize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
		while (m_textureImage.getHeight() > (unsigned int)texSize ||
			   m_textureImage.getWidth() > (unsigned int)texSize)
		{
			static bool warningEmitted = false;
			if (!warningEmitted)
			{
				warningEmitted = true;
				std::cerr
					<< "[mrpt::opengl::CRenderizableShaderTexturedTriangles] "
					   "**PERFORMACE WARNING**:\n"
					<< " Downsampling texture image of size "
					<< m_textureImage.getWidth() << "x"
					<< m_textureImage.getHeight()
					<< " since maximum allowed OpenGL texture size "
					   "(GL_MAX_TEXTURE_SIZE) is "
					<< texSize << "\n";
			}

			m_textureImage =
				m_textureImage.scaleHalf(mrpt::img::IMG_INTERP_LINEAR);
			m_textureImageAlpha =
				m_textureImageAlpha.scaleHalf(mrpt::img::IMG_INTERP_LINEAR);
		}

		const int width = m_textureImage.getWidth();
		const int height = m_textureImage.getHeight();

#ifdef TEXTUREOBJ_PROFILE_MEM_ALLOC
		{
			const std::string sSec = mrpt::format(
				"opengl_texture: load %ix%i %s %stransp", width, height,
				m_textureImage.isColor() ? "RGB" : "BW",
				m_enableTransparency ? "" : "no ");
			tim.enter(sSec.c_str());
		}
#endif

		if (m_enableTransparency)
		{
			ASSERT_(!m_textureImageAlpha.isColor());
			ASSERT_EQUAL_(
				m_textureImageAlpha.getWidth(), m_textureImage.getWidth());
			ASSERT_EQUAL_(
				m_textureImageAlpha.getHeight(), m_textureImage.getHeight());
		}

		// GL_LUMINANCE and GL_LUMINANCE_ALPHA were removed in OpenGL 3.1
		// Convert grayscale images into color:
		if (!m_textureImage.isColor())
			m_textureImage = m_textureImage.colorImage();

		// Color texture:
		if (m_enableTransparency)
		{
// Color texture WITH trans.
// --------------------------------------
#ifdef TEXTUREOBJ_PROFILE_MEM_ALLOC
			const std::string sSec = mrpt::format(
				"opengl_texture_alloc %ix%i (color,trans)", width, height);
			tim.enter(sSec.c_str());
#endif

			dataAligned = reserveDataBuffer(height * width * 4 + 512, data);

#ifdef TEXTUREOBJ_PROFILE_MEM_ALLOC
			tim.leave(sSec.c_str());
#endif

			for (int y = 0; y < height; y++)
			{
				unsigned char* ptrSrcCol = m_textureImage(0, y, 0);
				unsigned char* ptrSrcAlfa = m_textureImageAlpha(0, y);
				unsigned char* ptr = dataAligned + y * width * 4;

				for (int x = 0; x < width; x++)
				{
					*ptr++ = *ptrSrcCol++;
					*ptr++ = *ptrSrcCol++;
					*ptr++ = *ptrSrcCol++;
					*ptr++ = *ptrSrcAlfa++;
				}
			}

			// Prepare image data types:
			const GLenum img_type = GL_UNSIGNED_BYTE;
			// Reverse RGB <-> BGR order?
			const bool is_RGB_order =
				(m_textureImage.getChannelsOrder() == std::string("RGB"));
			const GLenum img_format = (is_RGB_order ? GL_RGBA : GL_BGRA);

			// Send image data to OpenGL:
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
			glTexImage2D(
				GL_TEXTURE_2D, 0 /*level*/, GL_RGBA8 /* RGB components */,
				width, height, 0 /*border*/, img_format, img_type, dataAligned);
			CHECK_OPENGL_ERROR();
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);	 // Reset
			CHECK_OPENGL_ERROR();

		}  // End of color texture WITH trans.
		else
		{
			// Color texture without transparency,
			// or with integrated RGBA alpha channel
			// --------------------------------------
			// Prepare image data types:
			const GLenum img_type = GL_UNSIGNED_BYTE;
			const int nBytesPerPixel = m_textureImage.channelCount();
			// Reverse RGB <-> BGR order?
			const bool is_RGB_order =
				(m_textureImage.getChannelsOrder() == std::string("RGB"));
			const GLenum img_format = [=]() {
				switch (nBytesPerPixel)
				{
					case 1: return GL_LUMINANCE;
					case 3: return (is_RGB_order ? GL_RGB : GL_BGR);
					case 4: return GL_BGRA;
				};
				THROW_EXCEPTION("Invalid texture image channel count.");
			}();

			// Send image data to OpenGL:
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			CHECK_OPENGL_ERROR();
			glPixelStorei(
				GL_UNPACK_ROW_LENGTH,
				m_textureImage.getRowStride() / nBytesPerPixel);
			CHECK_OPENGL_ERROR();
			glTexImage2D(
				GL_TEXTURE_2D, 0 /*level*/,
				nBytesPerPixel == 3 ? GL_RGB8 : GL_RGBA8 /* RGB components */,
				width, height, 0 /*border*/, img_format, img_type,
				m_textureImage.ptrLine<uint8_t>(0));
			CHECK_OPENGL_ERROR();
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);	 // Reset
			CHECK_OPENGL_ERROR();

		}  // End of color texture WITHOUT trans.

		// Was: m_texture_is_loaded = true;
		// Now this situation is represented by the optional m_glTexture having
		// a valid value.

#ifdef TEXTUREOBJ_PROFILE_MEM_ALLOC
		{
			const std::string sSec = mrpt::format(
				"opengl_texture: load %ix%i %s %stransp", width, height,
				m_textureImage.isColor() ? "RGB" : "BW",
				m_enableTransparency ? "" : "no ");
			tim.leave(sSec.c_str());
		}
#endif

#ifdef TEXTUREOBJ_USE_MEMPOOL
		// Before freeing the buffer in "data", donate my memory to the pool:
		if (!data.empty())
		{
			TMyMemPool* pool = TMyMemPool::getInstance();
			if (pool)
			{
				CRenderizableShaderTexturedTriangles_MemPoolParams mem_params;
				mem_params.len = data.size();

				auto* mem_block =
					new CRenderizableShaderTexturedTriangles_MemPoolData();
				data.swap(mem_block->data);

				pool->dump_to_pool(mem_params, mem_block);
			}
		}
#endif
	}
	catch (exception& e)
	{
		THROW_EXCEPTION(format(
			"m_glTextureName=%i\n%s", m_glTexture ? m_glTexture->name : 0,
			e.what()));
	}
	catch (...)
	{
		THROW_EXCEPTION("Runtime error!");
	}
#endif
}

CRenderizableShaderTexturedTriangles::~CRenderizableShaderTexturedTriangles()
{
	try
	{
		unloadTexture();
	}
	catch (const std::exception& e)
	{
		std::cerr
			<< "[~CRenderizableShaderTexturedTriangles] Ignoring exception: "
			<< mrpt::exception_to_str(e);
	}
}
void CRenderizableShaderTexturedTriangles::unloadTexture()
{
	if (!m_glTexture.has_value()) return;

	releaseTextureName(*m_glTexture);

	m_glTexture.reset();
}

void CRenderizableShaderTexturedTriangles::writeToStreamTexturedObject(
	mrpt::serialization::CArchive& out) const
{
	uint8_t ver = 2;

	out << ver;
	out << m_enableTransparency << m_textureInterpolate;
	out << m_textureImage;
	if (m_enableTransparency) out << m_textureImageAlpha;
	out << m_textureImageAssigned;
	out << m_enableLight << static_cast<uint8_t>(m_cullface);  // v2
}

void CRenderizableShaderTexturedTriangles::readFromStreamTexturedObject(
	mrpt::serialization::CArchive& in)
{
	uint8_t version;
	in >> version;

	switch (version)
	{
		case 0:
		case 1:
		case 2:
		{
			in >> m_enableTransparency >> m_textureInterpolate;
			in >> m_textureImage;
			if (m_enableTransparency)
			{
				in >> m_textureImageAlpha;
				assignImage(m_textureImage, m_textureImageAlpha);
			}
			else
			{
				assignImage(m_textureImage);
			}
			if (version >= 1) in >> m_textureImageAssigned;
			else
				m_textureImageAssigned = true;

			if (version >= 2)
			{
				in >> m_enableLight;
				m_cullface = static_cast<TCullFace>(in.ReadAs<uint8_t>());
			}
		}
		break;
		default: MRPT_THROW_UNKNOWN_SERIALIZATION_VERSION(version);
	};

	CRenderizable::notifyChange();
}

/** This class is a workaround to crashes and memory leaks caused by not
 * reserving and freeing opengl textures from the same thread. */
class TextureResourceHandler
{
   public:
	static TextureResourceHandler& Instance()
	{
		static TextureResourceHandler o;
		return o;
	}

	/// Return [textureName, textureUnit]
	std::pair<unsigned int, unsigned int> generateTextureID()
	{
#if MRPT_HAS_OPENGL_GLUT
		auto lck = mrpt::lockHelper(m_texturesMtx);

		processDestroyQueue();

		// Create one OpenGL texture
		GLuint textureID;
		glGenTextures(1, &textureID);
		CHECK_OPENGL_ERROR();
		m_textureReservedFrom[textureID] = std::this_thread::get_id();

		int foundUnit = -1;
		for (int i = 0; i < m_maxTextureUnits; i++)
			if (!m_occupiedTextureUnits.count(i))
			{
				foundUnit = i;
				break;
			}
		if (foundUnit < 0)
		{
			foundUnit = 0;
			std::cerr
				<< "[mrpt TextureResourceHandler] **WARNING**: Apparently "
				   "your program reached the maximum number of allowed "
				   "simultaneous OpenGL textures ("
				<< m_maxTextureUnits << ")" << std::endl;
		}
		else
		{
			m_occupiedTextureUnits.insert(foundUnit);
		}

		if (MRPT_OPENGL_VERBOSE)
			std::cout << "[mrpt generateTextureID] textureName:" << textureID
					  << " unit: " << foundUnit << std::endl;

		return {textureID, foundUnit};
#else
		THROW_EXCEPTION("This function needs OpenGL");
#endif
	}

	void releaseTextureID(unsigned int texName, unsigned int texUnit)
	{
#if MRPT_HAS_OPENGL_GLUT
		MRPT_START
		auto lck = mrpt::lockHelper(m_texturesMtx);

		if (MRPT_OPENGL_VERBOSE)
			std::cout << "[mrpt releaseTextureID] textureName: " << texName
					  << " unit: " << texUnit << std::endl;

		m_destroyQueue[m_textureReservedFrom.at(texName)].push_back(texName);
		processDestroyQueue();
		m_occupiedTextureUnits.erase(texUnit);
		MRPT_END
#endif
	}

   private:
	TextureResourceHandler()
	{
#if MRPT_HAS_OPENGL_GLUT
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_maxTextureUnits);
		if (MRPT_OPENGL_VERBOSE)
			std::cout << "[mrpt TextureResourceHandler] maxTextureUnits:"
					  << m_maxTextureUnits << std::endl;
#endif
	}

	void processDestroyQueue()
	{
#if MRPT_HAS_OPENGL_GLUT
		if (auto itLst = m_destroyQueue.find(std::this_thread::get_id());
			itLst != m_destroyQueue.end())
		{
			auto& lst = itLst->second;
			glDeleteTextures(lst.size(), lst.data());
			CHECK_OPENGL_ERROR();
			lst.clear();
		}
#endif
	}

#if MRPT_HAS_OPENGL_GLUT
	std::mutex m_texturesMtx;
	std::map<GLuint, std::thread::id> m_textureReservedFrom;
	std::map<std::thread::id, std::vector<GLuint>> m_destroyQueue;
	std::set<GLint> m_occupiedTextureUnits;
	GLint m_maxTextureUnits;
#endif
};

CRenderizableShaderTexturedTriangles::texture_name_unit_t
	CRenderizableShaderTexturedTriangles::getNewTextureNumber()
{
	CRenderizableShaderTexturedTriangles::texture_name_unit_t ret;
	const auto r = TextureResourceHandler::Instance().generateTextureID();
	ret.name = r.first;
	ret.unit = r.second;
	return ret;
}

void CRenderizableShaderTexturedTriangles::releaseTextureName(
	const texture_name_unit_t& t)
{
	TextureResourceHandler::Instance().releaseTextureID(t.name, t.unit);
}

const mrpt::math::TBoundingBox
	CRenderizableShaderTexturedTriangles::trianglesBoundingBox() const
{
	mrpt::math::TBoundingBox bb;

	if (m_triangles.empty()) return bb;

	bb.min = mrpt::math::TPoint3D(
		std::numeric_limits<double>::max(), std::numeric_limits<double>::max(),
		std::numeric_limits<double>::max());
	bb.max = mrpt::math::TPoint3D(
		-std::numeric_limits<double>::max(),
		-std::numeric_limits<double>::max(),
		-std::numeric_limits<double>::max());

	for (const auto& t : m_triangles)
	{
		keep_min(bb.min.x, t.x(0));
		keep_max(bb.max.x, t.x(0));
		keep_min(bb.min.y, t.y(0));
		keep_max(bb.max.y, t.y(0));
		keep_min(bb.min.z, t.z(0));
		keep_max(bb.max.z, t.z(0));

		keep_min(bb.min.x, t.x(1));
		keep_max(bb.max.x, t.x(1));
		keep_min(bb.min.y, t.y(1));
		keep_max(bb.max.y, t.y(1));
		keep_min(bb.min.z, t.z(1));
		keep_max(bb.max.z, t.z(1));

		keep_min(bb.min.x, t.x(2));
		keep_max(bb.max.x, t.x(2));
		keep_min(bb.min.y, t.y(2));
		keep_max(bb.max.y, t.y(2));
		keep_min(bb.min.z, t.z(2));
		keep_max(bb.max.z, t.z(2));
	}
	return bb;
}

/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#pragma once

#include <mrpt/config.h>
#include <mrpt/gui/CGlCanvasBase.h>
#include <mrpt/opengl/COpenGLScene.h>
#include <mrpt/opengl/opengl_fonts.h>

namespace mrpt
{
namespace gui
{
}
}  // namespace mrpt
// have wxWidgets libs

#if MRPT_HAS_WXWIDGETS

#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/busyinfo.h>
#include <wx/colordlg.h>
#include <wx/dcmemory.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/intl.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/progdlg.h>
#include <wx/string.h>
#include <wx/textdlg.h>

#if wxUSE_GLCANVAS && MRPT_HAS_OPENGL_GLUT

#include <wx/dcclient.h>
#include <wx/glcanvas.h>

// To avoid conflicts between Eigen & X11 headers
#ifdef Success
#undef Success
#endif

namespace mrpt
{
namespace gui
{
/** This class implements a OpenGL canvas, and it's used in
 * gui::CDisplayWindow3D and a number of standalone applications in the MRPT
 * project.
 *  There is a filter to control the user capability of moving the camera with
 * the mouse. See OnUserManuallyMovesCamera
 * \ingroup mrpt_gui_grp
 */
class CWxGLCanvasBase : public CGlCanvasBase, public wxGLCanvas
{
   public:
	CWxGLCanvasBase(
		wxWindow* parent, wxWindowID id = wxID_ANY,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize, long style = 0,
		const wxString& name = _T("CWxGLCanvasBase"));

	~CWxGLCanvasBase() override = default;

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnEnterWindow(wxMouseEvent& event);

	void OnChar(wxKeyEvent& event);

	void OnMouseDown(wxMouseEvent& event);
	void OnMouseMove(wxMouseEvent& event);
	void OnMouseUp(wxMouseEvent&);
	void OnMouseWheel(wxMouseEvent& event);

	void Render();
	void InitGL();

	/** Set the camera from a CPose3D, which defines the +X,+Y axis as image
	 * place RIGHT and UP dirctions, and -Z as towards the pointing direction.
	 */
	void setCameraPose(const mrpt::poses::CPose3D& camPose);

	/**  Methods that can be implemented in custom derived classes  */
	virtual void OnCharCustom([[maybe_unused]] wxKeyEvent& event) {}
	virtual void OnPreRender() {}
	virtual void OnPostRender() {}
	virtual void OnPostRenderSwapBuffers(
		[[maybe_unused]] double At, [[maybe_unused]] wxPaintDC& dc)
	{
	}

	virtual void OnRenderError([[maybe_unused]] const wxString& str) {}

	bool is_GL_context_created() const { return m_init; }

   protected:
	std::unique_ptr<wxGLContext> m_gl_context;
	bool m_gl_context_assigned = false;
	bool m_init = false;

	long m_Key = 0;
	unsigned long m_StartTime = 0;
	unsigned long m_LastTime = 0;
	unsigned long m_LastRedraw = 0;

	// Used to create the gl context at startup.
	void OnWindowCreation(wxWindowCreateEvent& ev);
	void swapBuffers() override;
	void preRender() override;
	void postRender() override;
	void renderError(const std::string& err_msg) override;

};	// end of class

}  // namespace gui
}  // namespace mrpt

#endif	// wxUSE_GLCANVAS
#endif	// MRPT_HAS_WXWIDGETS

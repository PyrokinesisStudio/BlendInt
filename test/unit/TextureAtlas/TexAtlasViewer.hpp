#ifndef _TEXATLAS_VIEWER_HPP_
#define _TEXATLAS_VIEWER_HPP_

#include <BlendInt/Gui/Widget.hpp>
#include <BlendInt/Gui/TextureAtlas.hpp>

namespace BI = BlendInt;

class TexAtlasViewer: public BI::Widget
{
public:

	TexAtlasViewer ();
	
	virtual ~TexAtlasViewer ();
	
protected:
	
	virtual void Update (const BI::UpdateRequest& request);

	virtual BI::ResponseType Draw (const BI::RedrawEvent& event);
	
private:
	
	GLuint m_vao;
	
	BI::TextureAtlasExt m_atlas;

	BI::RefPtr<BI::GLSLProgram> m_program;
	BI::RefPtr<BI::GLArrayBuffer> m_vbo;
	BI::RefPtr<BI::GLArrayBuffer> m_tbo;	// texture coords
	
	static const char* vertex_shader;
	static const char* fragment_shader;
};

#endif

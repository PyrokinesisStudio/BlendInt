/*
 * This file is part of BlendInt (a Blender-like Interface Library in
 * OpenGL).
 *
 * BlendInt (a Blender-like Interface Library in OpenGL) is free
 * software: you can redistribute it and/or modify it under the terms
 * of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * BlendInt (a Blender-like Interface Library in OpenGL) is
 * distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BlendInt.	 If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Contributor(s): Freeman Zhang <zhanggyb@gmail.com>
 */

#ifdef __UNIX__
#ifdef __APPLE__
#include <gl.h>
#include <glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#endif  // __UNIX__

#include <iostream>

#include <BlendInt/OpenGL/GLFramebuffer.hpp>

#include "ScreenBuffer.hpp"

namespace BlendInt {

	const char* ScreenBuffer::vertex_shader =
			"#version 120\n"
			"attribute vec3 coord3d;"
			"attribute vec2 texcoord;"
			"varying vec2 f_texcoord;"
			""
			"void main(void) {"
			"	gl_Position = gl_ModelViewProjectionMatrix * vec4(coord3d, 1.0);"
			"	f_texcoord = texcoord;"
			"}";

	const char* ScreenBuffer::fragment_shader =
			"varying vec2 f_texcoord;"
			"uniform sampler2D texture;"
			""
			"void main(void) {"
			"	gl_FragColor = texture2D(texture, f_texcoord);"
			"}";

	ScreenBuffer::ScreenBuffer()
	: Object(), m_texture(0), m_program(0), m_vbo(0), m_tbo(0), uniform_texture(-1), attribute_coord3d(-1), attribute_texcoord(-1)
	{
		InitOnce();
	}

	ScreenBuffer::~ScreenBuffer()
	{
		if(m_texture) delete m_texture;

		Object::Destroy(m_tbo);
		Object::Destroy(m_vbo);
		Object::Destroy(m_program);
	}

	void ScreenBuffer::Render()
	{
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		m_program->Use();

		glActiveTexture(GL_TEXTURE0);
		m_texture->Bind();

		glUniform1i(uniform_texture, 0);

		glEnableVertexAttribArray(attribute_coord3d);
		m_vbo->Bind();
		glVertexAttribPointer(
				attribute_coord3d,
				3,
				GL_FLOAT,
				GL_FALSE,
				0,
				0
				);

		glEnableVertexAttribArray(attribute_texcoord);
		m_tbo->Bind();
		glVertexAttribPointer(
				attribute_texcoord,
				2,
				GL_FLOAT,
				GL_FALSE,
				0,
				0
				);

		m_vbo->Bind();

		glDrawArrays(GL_POLYGON, 0, 4);

		glDisableVertexAttribArray(attribute_texcoord);
		glDisableVertexAttribArray(attribute_coord3d);

		m_vbo->Reset();
		m_texture->Reset();
		m_program->Reset();

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void ScreenBuffer::InitOnce()
	{
		m_texture = new GLTexture2D;

		m_vbo = new GLArrayBuffer;
		Retain(m_vbo);

		m_vbo->Generate();

		float vertices[] = {
			0.0, 0.0, 0.0,
			0.0, 0.0, 0,0,
			0.0, 0.0, 0.0,
			0.0, 0.0, 0.0
		};

		m_vbo->Bind();
		m_vbo->SetData(4, sizeof(float) * 3, vertices);
		m_vbo->Reset();

		m_program = new GLSLProgram;
		Retain(m_program);

		m_program->Create();
		m_program->AttachShaderPair(vertex_shader, fragment_shader);

		if(m_program->Link()) {
			m_program->Use();

			attribute_coord3d = m_program->GetAttributeLocation("coord3d");
			attribute_texcoord = m_program->GetAttributeLocation("texcoord");
			uniform_texture = m_program->GetUniformLocation("texture");

			if(attribute_texcoord == -1 || attribute_coord3d == -1 || uniform_texture == -1) {
				std::cerr << "Fail to get attribute" << std::endl;
			}

			m_program->Reset();
		}

		m_tbo = new GLArrayBuffer;
		Retain(m_tbo);

		GLfloat texcoords[] = {
				0.0, 0.0,
				1.0, 0.0,
				1.0, 1.0,
				0.0, 1.0
		};

		m_tbo->Generate();
		m_tbo->Bind();
		m_tbo->SetData(4, sizeof(GLfloat) * 2, texcoords);
		m_tbo->Reset();
	}

	void ScreenBuffer::Resize (float width, float height, float depth)
	{
		float vertices[] = {
			0.0, 0.0, depth,
			width, 0.0, depth,
			width, height, depth,
			0.0, height, depth
		};

		m_vbo->Bind();
		m_vbo->UpdateData(vertices, 4 * sizeof(float) * 3);
		m_vbo->Reset();
	}

	void ScreenBuffer::SaveToFile (const char* filename)
	{
		m_texture->WriteToFile(filename);
	}


}

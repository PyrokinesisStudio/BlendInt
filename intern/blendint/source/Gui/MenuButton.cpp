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
#include <gl3.h>
#include <gl3ext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#endif	// __UNIX__

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <BlendInt/Gui/MenuButton.hpp>
#include <BlendInt/Gui/Context.hpp>

#include <BlendInt/Stock/Theme.hpp>
#include <BlendInt/Stock/Shaders.hpp>

namespace BlendInt {

    using Stock::Shaders;

	MenuButton::MenuButton (const String& text)
	: AbstractButton(), vao_(0)
	{
		InitializeMenuButton(text);
	}
	
	MenuButton::~MenuButton ()
	{
		glDeleteVertexArrays(1, &vao_);
	}
	
	void MenuButton::PerformSizeUpdate (const SizeUpdateRequest& request)
	{
		if(request.target() == this) {

			UpdateTextPosition(*request.size(), round_type(),
			        round_radius(), text());

            set_size(*request.size());

            std::vector<GLfloat> inner_verts;
            AbstractInteractiveForm::GenerateVertices(size(), 0.f, round_type(), round_radius(), &inner_verts, 0);

			inner_.bind();
			inner_.set_sub_data(0, sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);
            inner_.reset();

			Refresh();
		}

		if(request.source() == this) {
			ReportSizeUpdate(request);
		}
	}

	void MenuButton::PerformRoundTypeUpdate (int round_type)
	{
			UpdateTextPosition(size(), round_type, round_radius(),
			        text());

            set_round_type(round_type);

            std::vector<GLfloat> inner_verts;
            GenerateVertices(size(), 0.f, round_type, round_radius(), &inner_verts, 0);

            inner_.bind();
            inner_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);
            inner_.reset();

			Refresh();
	}

	void MenuButton::PerformRoundRadiusUpdate (float radius)
	{
			UpdateTextPosition(size(), round_type(), radius,
			        text());

			set_round_radius(radius);

            std::vector<GLfloat> inner_verts;
            GenerateVertices(size(), 0.f, round_type(), round_radius(), &inner_verts, 0);

            inner_.bind();
            inner_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);
            inner_.reset();

            Refresh();
	}

	ResponseType MenuButton::Draw (Profile& profile)
	{
		if (hover()) {

			Shaders::instance->widget_inner_program()->use();

			glUniform1i(Shaders::instance->location(Stock::WIDGET_INNER_GAMMA), 0);
			glUniform4fv(Shaders::instance->location(Stock::WIDGET_INNER_COLOR), 1, Theme::instance->menu_item().inner_sel.data());

			glBindVertexArray(vao_);
			glDrawArrays(GL_TRIANGLE_FAN, 0,
							GetOutlineVertices(round_type()) + 2);
			glBindVertexArray(0);

            GLSLProgram::reset();

		}

		if(text().size()) {
			font().Print(0.f, 0.f, text(), text_length(), 0);
		}

		return Accept;
	}

	void MenuButton::SetMenu (const RefPtr<Menu>& menu)
	{
		m_menu = menu;
	}

	void MenuButton::InitializeMenuButton (const String& text)
	{
		set_round_type(RoundAll);
		set_text(text);

		int h = font().GetHeight();

		if(text.empty()) {
			set_size(h + round_radius() * 2 + default_padding.hsum(),
							h + default_padding.vsum());
		} else {
			set_text_length(text.length());
			Rect text_outline = font().GetTextOutline(text);

			int width = text_outline.width() + round_radius() * 2 + default_padding.hsum();
			int height = h + default_padding.vsum();

			set_size(width, height);

			set_pen((width - text_outline.width()) / 2,
							(height - font().GetHeight()) / 2
											+ std::abs(font().GetDescender()));
		}

		glGenVertexArrays(1, &vao_);

        std::vector<GLfloat> inner_verts;
        GenerateVertices(size(), 0.f, round_type(), round_radius(), &inner_verts, 0);

		glBindVertexArray(vao_);
		inner_.generate();
		inner_.bind();
		inner_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);

		glEnableVertexAttribArray(Shaders::instance->location(Stock::WIDGET_INNER_COORD));
		glVertexAttribPointer(Shaders::instance->location(Stock::WIDGET_INNER_COORD), 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);
		GLArrayBuffer::reset();
	}

} /* namespace BlendInt */
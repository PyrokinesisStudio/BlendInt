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
#endif  // __UNIX__

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <BlendInt/Gui/TimeRuler.hpp>

#include <BlendInt/Gui/Context.hpp>

namespace BlendInt {

	TimeRuler::TimeRuler()
	: Widget()
	{
		set_size(200, 14);
		set_round_type(RoundAll);
		set_round_radius(7.f);

		glGenVertexArrays(2, vao_);

		std::vector<GLfloat> inner_verts;
		std::vector<GLfloat> outer_verts;

		if(Context::theme->scroll().shaded) {

			short shadetop = Context::theme->scroll().shadetop;
			short shadedown = Context::theme->scroll().shadedown;

			GenerateRoundedVertices(
					Vertical,
					shadetop,
					shadedown,
					&inner_verts,
					&outer_verts);
		} else {
			GenerateRoundedVertices(
					&inner_verts,
					&outer_verts);
		}

		buffer_.generate();

		glBindVertexArray(vao_[0]);

		buffer_.bind(0);
		buffer_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);

		glEnableVertexAttribArray(Context::shaders->location(Shaders::WIDGET_INNER_COORD));
		glVertexAttribPointer(Context::shaders->location(Shaders::WIDGET_INNER_COORD), 3, GL_FLOAT, GL_FALSE, 0,
		        BUFFER_OFFSET(0));

		glBindVertexArray(vao_[1]);
		buffer_.bind(1);
		buffer_.set_data(sizeof(GLfloat) * outer_verts.size(), &outer_verts[0]);

		glEnableVertexAttribArray(Context::shaders->location(Shaders::WIDGET_OUTER_COORD));
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);
		buffer_.reset();
	}

	TimeRuler::~TimeRuler()
	{
	}

	bool TimeRuler::IsExpandX() const
	{
		return true;
	}

	Size TimeRuler::GetPreferredSize() const
	{
		return Size(500, 14);
	}

	void TimeRuler::PerformSizeUpdate(const SizeUpdateRequest& request)
	{
		if(request.target() == this) {

			float radius = std::min(request.size()->width(), request.size()->height()) / 2.f;

			set_size(*request.size());
			set_round_radius(radius);

			std::vector<GLfloat> inner_verts;
			std::vector<GLfloat> outer_verts;

			if(Context::theme->scroll().shaded) {

				short shadetop = Context::theme->scroll().shadetop;
				short shadedown = Context::theme->scroll().shadedown;

				GenerateRoundedVertices(
						Vertical,
						shadetop,
						shadedown,
						&inner_verts,
						&outer_verts);
			} else {
				GenerateRoundedVertices(
						&inner_verts,
						&outer_verts);
			}

			buffer_.bind(0);
			buffer_.set_sub_data(0, sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);

			buffer_.bind(1);
			buffer_.set_sub_data(0, sizeof(GLfloat) * outer_verts.size(), &outer_verts[0]);

			buffer_.reset();

			RequestRedraw();
		}

		if(request.source() == this) {
			ReportSizeUpdate(request);
		}
	}

	ResponseType TimeRuler::Draw(const Context* context)
	{
		Context::shaders->widget_inner_program()->use();

		glUniform1i(Context::shaders->location(Shaders::WIDGET_INNER_GAMMA), 0);
		glUniform4fv(Context::shaders->location(Shaders::WIDGET_INNER_COLOR), 1,
				Context::theme->scroll().item.data());

		glBindVertexArray(vao_[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, GetOutlineVertices(round_type()) + 2);

		Context::shaders->widget_outer_program()->use();

		glUniform2f(Context::shaders->location(Shaders::WIDGET_OUTER_POSITION),
		        0.f, 0.f);
		glUniform4fv(Context::shaders->location(Shaders::WIDGET_OUTER_COLOR), 1,
		        Context::theme->scroll().outline.data());

		glBindVertexArray(vao_[1]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0,
		        GetOutlineVertices(round_type()) * 2 + 2);

		if (emboss()) {
			glUniform4f(Context::shaders->location(Shaders::WIDGET_OUTER_COLOR), 1.f,
			        1.f, 1.f, 0.16f);
			glUniform2f(Context::shaders->location(Shaders::WIDGET_OUTER_POSITION),
					0.f, 0.f - 1.f);
			glDrawArrays(GL_TRIANGLE_STRIP, 0,
			        GetHalfOutlineVertices(round_type()) * 2);
		}

		glBindVertexArray(0);
		GLSLProgram::reset();

		return Finish;
	}

}

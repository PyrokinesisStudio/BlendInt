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
 * License along with BlendInt.  If not, see
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

#include <BlendInt/Gui/VertexTool.hpp>
#include <BlendInt/Gui/Menu.hpp>
#include <BlendInt/Stock/Theme.hpp>
#include <BlendInt/Stock/ShaderManager.hpp>

namespace BlendInt {

	int Menu::DefaultMenuItemHeight = 16;
	int Menu::DefaultIconSpace = 4;
	int Menu::DefaultShortcutSpace = 20;

	Menu::Menu ()
	: Widget(), m_highlight(0), m_inner_buffer(0), m_outer_buffer(0), m_highlight_buffer(0)
	{
		set_size (20, 20);
		set_drop_shadow(true);
		InitializeMenu();
	}

	Menu::~Menu ()
	{
		glDeleteVertexArrays(1, &m_vao);
	}

	void Menu::SetTitle(const String& title)
	{
		m_title = title;
	}

	void Menu::AddAction(const String& text)
	{
		RefPtr<Action> item = Action::Create(text);

		AddAction(item);
	}

	void Menu::AddAction(const String& text, const String& shortcut)
	{
		RefPtr<Action> item = Action::Create(text, shortcut);

		AddAction(item);
	}

	void Menu::AddAction(const RefPtr<Icon>& icon, const String& text)
	{
		RefPtr<Action> item = Action::Create(icon, text);

		AddAction(item);
	}

	void Menu::AddAction(const RefPtr<Icon>& icon, const String& text, const String& shortcut)
	{
		RefPtr<Action> item = Action::Create(icon, text, shortcut);

		AddAction(item);
	}

	void Menu::AddAction(const RefPtr<Action>& item)
	{
		int width = 0;

		width = item->GetTextLength(m_font);

		width += 16 + DefaultIconSpace + DefaultShortcutSpace;

		Size s;

		if(m_list.size()) {
			s.set_width(std::max(size().width(), round_corner_radius() * 2 + width));
			s.set_height(size().height() + DefaultMenuItemHeight);
		} else {
			s.set_width(round_corner_radius() * 2 + width);
			s.set_height(round_corner_radius() * 2 + DefaultMenuItemHeight);
		}

		Resize(s);

		m_list.push_back(item);
	}

	ResponseType Menu::MouseMoveEvent(const MouseEvent& event)
	{
		unsigned int orig = m_highlight;

		if(!Contain(event.position())) {
			m_highlight = 0;

			if(orig != m_highlight) {
				Refresh();
			}
			return Accept;
		}

		if(!m_list.size()) {
			m_highlight = 0;
			if(orig != m_highlight) {
				Refresh();
			}
			return Accept;
		}

		m_highlight = GetHighlightNo(static_cast<int>(event.position().y()));

		if(orig != m_highlight) {
			Refresh();
		}
		return AcceptAndBreak;
	}

	ResponseType Menu::MousePressEvent (const MouseEvent& event)
	{
		/*
		if(!m_menubin->size()) {
			return Accept;
		}

		m_triggered.fire(m_menubin->GetMenuItem(m_highlight - 1));
		*/
		if(m_highlight > 0) {

			Action* item = m_list[m_highlight - 1].get();
			m_triggered.fire(item);
		}

		return Accept;
	}

	ResponseType Menu::MouseReleaseEvent (const MouseEvent& event)
	{
		return Accept;
	}

	void Menu::UpdateGeometry(const WidgetUpdateRequest& request)
	{
		switch (request.type()) {

			case WidgetSize: {
				const Size* size_p = static_cast<const Size*>(request.data());
				VertexTool tool;
				tool.Setup(*size_p, DefaultBorderWidth(), round_corner_type(),
								round_corner_radius());
				tool.UpdateInnerBuffer(m_inner_buffer.get());
				tool.UpdateOuterBuffer(m_outer_buffer.get());
				ResetHighlightBuffer(size_p->width());
				break;
			}

			case WidgetRoundCornerType: {
				const int* type_p = static_cast<const int*>(request.data());
				VertexTool tool;
				tool.Setup(size(), DefaultBorderWidth(), *type_p,
								round_corner_radius());
				tool.UpdateInnerBuffer(m_inner_buffer.get());
				tool.UpdateOuterBuffer(m_outer_buffer.get());
				break;
			}

			case WidgetRoundCornerRadius: {
				const int* radius_p =
								static_cast<const int*>(request.data());
				VertexTool tool;
				tool.Setup(size(), DefaultBorderWidth(), round_corner_type(),
								*radius_p);
				tool.UpdateInnerBuffer(m_inner_buffer.get());
				tool.UpdateOuterBuffer(m_outer_buffer.get());
				break;
			}

			default:
				Widget::UpdateGeometry(request);
		}

	}

	ResponseType Menu::Draw (const RedrawEvent& event)
	{
		using std::deque;

		glm::vec3 pos((float) position().x(), (float) position().y(),
						(float) z());
		glm::mat4 mvp = glm::translate(event.projection_matrix() * event.view_matrix(), pos);

		glBindVertexArray(m_vao);

		RefPtr<GLSLProgram> program = ShaderManager::instance->default_triangle_program();

		program->Use();

		program->SetUniformMatrix4fv("MVP", 1, GL_FALSE, glm::value_ptr(mvp));
		program->SetUniform1i("AA", 0);
		program->SetVertexAttrib4fv("Color", Theme::instance->menu().inner.data());

		glEnableVertexAttribArray(0);

		DrawTriangleFan(0, m_inner_buffer.get());

		program->SetVertexAttrib4fv("Color", Theme::instance->menu().outline.data());
		program->SetUniform1i("AA", 1);

		DrawTriangleStrip(0, m_outer_buffer.get());

		if(m_highlight) {
			program->SetUniform1i("AA", 0);

			glEnableVertexAttribArray(1);

			glm::mat4 h_mvp = glm::translate(mvp, glm::vec3(0.f, size().height() - round_corner_radius() - static_cast<float>(DefaultMenuItemHeight * m_highlight), 0.f));
			program->SetUniformMatrix4fv("MVP", 1, GL_FALSE, glm::value_ptr(h_mvp));

			DrawShadedTriangleFan(0, 1, m_highlight_buffer.get());

			glDisableVertexAttribArray(1);
		}

		glDisableVertexAttribArray(0);

		program->Reset();

		glBindVertexArray(0);

		float h = size().height() - round_corner_radius();

		int advance = 0;
		for(deque<RefPtr<Action> >::iterator it = m_list.begin(); it != m_list.end(); it++)
		{
			h = h - DefaultMenuItemHeight;

			if((*it)->icon()) {
				(*it)->icon()->Draw(mvp, 8, h + 8, 16, 16);
			}
			advance = m_font.Print(mvp, 16 + DefaultIconSpace, h - m_font.GetDescender(), (*it)->text());
			m_font.Print(mvp, 16 + DefaultIconSpace + advance + DefaultShortcutSpace, h - m_font.GetDescender(), (*it)->shortcut());
		}

		return Accept;
	}

	void Menu::ResetHighlightBuffer (int width)
	{
		Size size(width, DefaultMenuItemHeight);

		VertexTool tool;
		tool.Setup(size,
				DefaultBorderWidth(),
				RoundNone,
				0,
				Theme::instance->menu_item().inner_sel,
				Vertical,
				Theme::instance->menu_item().shadetop,
				Theme::instance->menu_item().shadedown
				);
		tool.UpdateInnerBuffer(m_highlight_buffer.get());
	}
	
	void Menu::RemoveAction (size_t index)
	{
	}
	
	void Menu::RemoveAction (const RefPtr<Action>& item)
	{
	}
	
	ResponseType Menu::FocusEvent (bool focus)
	{
		DBG_PRINT_MSG("focus %s", focus ? "on" : "off");

		if(focus) {
			SetVisible(true);
		} else {
			SetVisible(false);
		}

		Refresh();

		return Ignore;
	}

	unsigned int Menu::GetHighlightNo(int y)
	{
		int h = position().y() + size().height() - y;

		if(h < round_corner_radius() || h > (size().height() - round_corner_radius())) {
			return 0;
		}

		return (h - round_corner_radius()) / (size().height() / m_list.size()) + 1;
	}

	void Menu::InitializeMenu ()
	{
		glGenVertexArrays(1, &m_vao);

		VertexTool tool;

		tool.Setup(size(), DefaultBorderWidth(), round_corner_type(), round_corner_radius());
		m_inner_buffer = tool.GenerateInnerBuffer();
		m_outer_buffer = tool.GenerateOuterBuffer();

		m_highlight_buffer.reset(new GLArrayBuffer);

		ResetHighlightBuffer(20);
	}

}


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

#include <gui/color-button.hpp>
#include <gui/abstract-window.hpp>

namespace BlendInt {

  ColorButton::ColorButton ()
      : AbstractButton(), selector_(0)
  {
    set_round_type(RoundAll);

    Font font;	// default font
    int w = 80;
    int h = font.height();

    set_size(w + pixel_size(kPadding.hsum()), h + pixel_size(kPadding.vsum()));

    color0_.set_red(0.3f);
    color0_.set_blue(0.8f);
    color0_.set_green(0.2f);

    color1_.set_red(0.3f);
    color1_.set_blue(0.8f);
    color1_.set_green(0.2f);
    color1_.set_alpha(0.5f);

    InitializeColorButton();

    events()->connect(clicked(), this, &ColorButton::OnClick);
  }

  ColorButton::~ColorButton ()
  {
    glDeleteVertexArrays(2, vao_);
  }

  void ColorButton::SetColor (const Color& color)
  {
    color0_ = color;
    color0_.set_alpha(1.f);
    color1_ = color;
    RequestRedraw();
  }

  void ColorButton::PerformSizeUpdate (const SizeUpdateRequest& request)
  {
    if (request.target() == this) {

      set_size(*request.size());

      std::vector<GLfloat> inner_verts;
      std::vector<GLfloat> outer_verts;

      GenerateRoundedVertices(&inner_verts, &outer_verts);
      vbo_.bind(0);
      vbo_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);
      vbo_.bind(1);
      vbo_.set_data(sizeof(GLfloat) * outer_verts.size(), &outer_verts[0]);
      vbo_.reset();

      RequestRedraw();
    }

    if (request.source() == this) {
      ReportSizeUpdate(request);
    }
  }

  void ColorButton::PerformRoundTypeUpdate (int round_type)
  {
    set_round_type(round_type);

    std::vector<GLfloat> inner_verts;
    std::vector<GLfloat> outer_verts;

    GenerateRoundedVertices(&inner_verts, &outer_verts);
    vbo_.bind(0);
    vbo_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);
    vbo_.bind(1);
    vbo_.set_data(sizeof(GLfloat) * outer_verts.size(), &outer_verts[0]);
    vbo_.reset();

    RequestRedraw();
  }

  void ColorButton::PerformRoundRadiusUpdate (float radius)
  {
    set_round_radius(radius);

    std::vector<GLfloat> inner_verts;
    std::vector<GLfloat> outer_verts;

    GenerateRoundedVertices(&inner_verts, &outer_verts);
    vbo_.bind(0);
    vbo_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);
    vbo_.bind(1);
    vbo_.set_data(sizeof(GLfloat) * outer_verts.size(), &outer_verts[0]);
    vbo_.reset();

    RequestRedraw();
  }

  Response ColorButton::Draw (AbstractWindow* context)
  {
    float x = context->active_frame()->GetRelativePosition(this).x()
        - context->viewport_origin().x();

    int outline_vertices = GetOutlineVertices(round_type());

    AbstractWindow::shaders()->widget_split_inner_program()->use();

    glUniform1f(
        AbstractWindow::shaders()->location(Shaders::WIDGET_SPLIT_INNER_PARTING),
        x + size().width() / 2.f);
    glUniform4fv(
        AbstractWindow::shaders()->location(Shaders::WIDGET_SPLIT_INNER_COLOR0),
        1, color0_.data());
    glUniform4fv(
        AbstractWindow::shaders()->location(Shaders::WIDGET_SPLIT_INNER_COLOR1),
        1, color1_.data());

    glUniform1i(
        AbstractWindow::shaders()->location(Shaders::WIDGET_SPLIT_INNER_GAMMA),
        0);

    glBindVertexArray(vao_[0]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, outline_vertices + 2);

    AbstractWindow::shaders()->widget_outer_program()->use();

    glUniform2f(
        AbstractWindow::shaders()->location(Shaders::WIDGET_OUTER_POSITION), 0.f,
        0.f);
    glUniform4fv(AbstractWindow::shaders()->location(Shaders::WIDGET_OUTER_COLOR),
                 1, AbstractWindow::theme()->regular().outline.data());

    glBindVertexArray(vao_[1]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, outline_vertices * 2 + 2);

    if (emboss()) {
      glUniform4f(
          AbstractWindow::shaders()->location(Shaders::WIDGET_OUTER_COLOR), 1.0f,
          1.0f, 1.0f, 0.16f);

      glUniform2f(
          AbstractWindow::shaders()->location(Shaders::WIDGET_OUTER_POSITION),
          0.f, -1.f);

      glDrawArrays(GL_TRIANGLE_STRIP, 0,
                   GetHalfOutlineVertices(round_type()) * 2);
    }

    DrawIconText();

    return Finish;
  }

  Size ColorButton::GetPreferredSize () const
  {
    Size s = AbstractButton::GetPreferredSize();
    if (s.width() < 80) s.set_width(80);

    return s;
  }

  bool ColorButton::IsExpandX () const
  {
    return true;
  }

  void ColorButton::InitializeColorButton ()
  {
    std::vector<GLfloat> inner_verts;
    std::vector<GLfloat> outer_verts;

    GenerateRoundedVertices(&inner_verts, &outer_verts);

    vbo_.generate();

    glGenVertexArrays(2, vao_);
    glBindVertexArray(vao_[0]);

    vbo_.bind(0);
    vbo_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);

    glEnableVertexAttribArray(AttributeCoord);
    glVertexAttribPointer(AttributeCoord, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(vao_[1]);
    vbo_.bind(1);
    vbo_.set_data(sizeof(GLfloat) * outer_verts.size(), &outer_verts[0]);

    glEnableVertexAttribArray(AttributeCoord);
    glVertexAttribPointer(AttributeCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    vbo_.reset();
  }

  void ColorButton::OnClick (AbstractButton* sender)
  {
    if (selector_ == 0) {
      selector_ = new ColorSelector;
      events()->connect(selector_->destroyed(), this,
                        &ColorButton::OnSelectorDestroyed);
      AbstractWindow* win = AbstractWindow::GetWindow(this);
      win->AddFrame(selector_);

      Point pos = win->GetGlobalCursorPosition();

      if ((pos.y() + selector_->size().height()) > win->size().height()) {
        pos.set_y(win->size().height() - selector_->size().height());
      }

      selector_->MoveTo(pos);
    }
  }

  void ColorButton::OnSelectorDestroyed (AbstractFrame* sender)
  {
    assert(sender == selector_);
    selector_ = 0;
  }

}
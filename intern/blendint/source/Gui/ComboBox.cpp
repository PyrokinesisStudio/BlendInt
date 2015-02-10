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

#include <math.h>

#include <BlendInt/OpenGL/GLHeader.hpp>

#include <BlendInt/Gui/AbstractWindow.hpp>
#include <BlendInt/Gui/Dialog.hpp>
#include <BlendInt/Gui/FileSystemModel.hpp>

#include <BlendInt/Gui/ComboBox.hpp>

namespace BlendInt {

	Margin ComboBox::default_combobox_padding = Margin(2, 2, 2, 2);

	ComboBox::ComboBox ()
	: Widget(),
	  status_down_(false)//,
	//popup_(0)
	{
		set_round_type(RoundAll);

		int h = font_.height();

		set_size(
		        h + round_radius() * 2 + default_combobox_padding.hsum() + 100,
		        h + default_combobox_padding.vsum());

		InitializeComboBox();
	}

	ComboBox::~ComboBox ()
	{
		glDeleteVertexArrays(2, vaos_);
	}

	Size ComboBox::GetPreferredSize () const
	{
		Size preferred_size;

		int radius_plus = 0;

		if((round_type() & RoundTopLeft) || (round_type() & RoundBottomLeft)) {
			radius_plus += round_radius();
		}

		if((round_type() & RoundTopRight) || (round_type() & RoundBottomRight)) {
			radius_plus += round_radius();
		}

		int max_font_height = font_.height();

		preferred_size.set_height(max_font_height + default_combobox_padding.vsum());	// top padding: 2, bottom padding: 2

		if (text_.empty()) {
			preferred_size.set_width(
							max_font_height + default_combobox_padding.hsum()
											+ radius_plus + 100);
		} else {
			size_t width = font_.GetTextWidth(text_);
			preferred_size.set_width(width
							+ default_combobox_padding.hsum()
							+ radius_plus);	// left padding: 2, right padding: 2
		}

		return preferred_size;
	}

	bool ComboBox::IsExpandX() const
	{
		return true;
	}

	void ComboBox::SetModel (const RefPtr<AbstractItemModel>& model)
	{
		model_ = model;
		//if(list_) {
		//	list_->SetModel(model_);
		//}
	}

	void ComboBox::PerformSizeUpdate (const SizeUpdateRequest& request)
	{
		if(request.target() == this) {

			set_size(*request.size());

			std::vector<GLfloat> inner_verts;
			std::vector<GLfloat> outer_verts;

			if (AbstractWindow::theme->menu().shaded) {
				GenerateRoundedVertices(Vertical,
						AbstractWindow::theme->menu().shadetop,
						AbstractWindow::theme->menu().shadedown,
						&inner_verts,
						&outer_verts);
			} else {
				GenerateRoundedVertices(&inner_verts, &outer_verts);
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

	void ComboBox::PerformRoundTypeUpdate (int round_type)
	{
		set_round_type(round_type);

		std::vector<GLfloat> inner_verts;
		std::vector<GLfloat> outer_verts;

		if (AbstractWindow::theme->menu().shaded) {
			GenerateRoundedVertices(Vertical,
						AbstractWindow::theme->menu().shadetop,
						AbstractWindow::theme->menu().shadedown,
						&inner_verts,
						&outer_verts);
		} else {
			GenerateRoundedVertices(&inner_verts, &outer_verts);
		}

		buffer_.bind(0);
		buffer_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);
		buffer_.bind(1);
		buffer_.set_data(sizeof(GLfloat) * outer_verts.size(), &outer_verts[0]);
		buffer_.reset();

		RequestRedraw();
	}

	void ComboBox::PerformRoundRadiusUpdate (float radius)
	{
		set_round_radius(radius);

		std::vector<GLfloat> inner_verts;
		std::vector<GLfloat> outer_verts;

		if (AbstractWindow::theme->menu().shaded) {
			GenerateRoundedVertices(Vertical,
					AbstractWindow::theme->menu().shadetop,
					AbstractWindow::theme->menu().shadedown,
					&inner_verts,
					&outer_verts);
		} else {
			GenerateRoundedVertices(&inner_verts, &outer_verts);
		}

		buffer_.bind(0);
		buffer_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);
		buffer_.bind(1);
		buffer_.set_data(sizeof(GLfloat) * outer_verts.size(), &outer_verts[0]);
		buffer_.reset();

		RequestRedraw();
	}

	Response ComboBox::Draw(AbstractWindow* context)
	{
		AbstractWindow::shaders->widget_inner_program()->use();

		glUniform4fv(AbstractWindow::shaders->location(Shaders::WIDGET_INNER_COLOR), 1,
				AbstractWindow::theme->menu().inner.data());

		if (status_down_) {
			glUniform1i(AbstractWindow::shaders->location(Shaders::WIDGET_INNER_GAMMA), 20);
		} else {
			if (hover()) {
				glUniform1i(AbstractWindow::shaders->location(Shaders::WIDGET_INNER_GAMMA),
						15);
			} else {
				glUniform1i(AbstractWindow::shaders->location(Shaders::WIDGET_INNER_GAMMA),
						0);
			}
		}

		glBindVertexArray(vaos_[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, GetOutlineVertices(round_type()) + 2);

		AbstractWindow::shaders->widget_outer_program()->use();

		glUniform4fv(AbstractWindow::shaders->location(Shaders::WIDGET_OUTER_COLOR), 1,
				AbstractWindow::theme->menu().outline.data());
		glUniform2f(AbstractWindow::shaders->location(Shaders::WIDGET_OUTER_POSITION), 0.f, 0.f);

		glBindVertexArray(vaos_[1]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0,
				GetOutlineVertices(round_type()) * 2 + 2);

//		if (emboss()) {
//			glUniform4f(AbstractWindow::shaders->location(Shaders::WIDGET_OUTER_COLOR), 1.0f,
//			        1.0f, 1.0f, 0.16f);
//			glUniform2f(AbstractWindow::shaders->location(Shaders::WIDGET_OUTER_POSITION),
//			        0.f, - 1.f);
//			glDrawArrays(GL_TRIANGLE_STRIP, 0,
//			        GetHalfOutlineVertices(round_type()) * 2);
//		}

		//RefPtr<VertexIcon> icon = Icons::instance->icon_menu();

		//glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(1.15, 1.15, 1.15));
		//glm::mat4 rotate = glm::rotate(glm::mat4(1.0), (glm::mediump_float)(M_PI * 1.5), glm::vec3(0.0, 0.0, 1.0));
		//glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(icon->size().width()/2.f, icon->size().height()/2.f, 0.0));

		//icon->Draw(mvp * translate * rotate * scale);

		float x = size().width() - AbstractWindow::icons->menu()->size().width()/2.f;
		float y = size().height()/2.f;

		AbstractWindow::icons->menu()->Draw(x, y, Color(0xEFEFEFFF));

		return Finish;
	}
	
	Response ComboBox::PerformMousePress (AbstractWindow* context)
	{
		status_down_ = true;

		/*
		if(popup_) {
			delete popup_;
			popup_ = 0;
			SetRoundType(RoundAll);
		} else {
			ListView* list = Manage(new ListView);

			if(model_) {
				list->SetModel(model_);
			}

			list->Resize(200, list->size().height());

			popup_ = Manage(new PopupFrame);
			popup_->Resize(list->size());
			popup_->AddWidget(list);

			events()->connect(popup_->destroyed(), this, &ComboBox::OnPopupListDestroyed);

			Point pos = context->active_frame()->GetAbsolutePosition(this);

			int top = pos.y() + size().height() + popup_->size().height();
			int bottom = pos.y() - popup_->size().height();

			if(top <= context->size().height()) {
				popup_->MoveTo(pos.x(), pos.y() + size().height());
				SetRoundType(RoundBottomLeft | RoundBottomRight);
			} else {

				if(bottom >= 0) {
					popup_->MoveTo(pos.x(), pos.y() - popup_->size().height());
					SetRoundType(RoundTopLeft | RoundTopRight);
				} else {

					int diff = top - context->size().height() + bottom;
					if(diff <= 0) {
						popup_->MoveTo(pos.x(), pos.y() + size().height());
						SetRoundType(RoundBottomLeft | RoundBottomRight);
					} else {
						popup_->MoveTo(pos.x(), pos.y() - popup_->size().height());
						SetRoundType(RoundTopLeft | RoundTopRight);
					}

				}

			}

			context->AddFrame(popup_);
		}
		*/

		RequestRedraw();

		return Finish;
	}
	
	Response ComboBox::PerformMouseRelease (AbstractWindow* context)
	{
		status_down_ = false;

		RequestRedraw();
		return Finish;
	}
	
	void ComboBox::PerformHoverIn(AbstractWindow* context)
	{
		RequestRedraw();
	}

	void ComboBox::PerformHoverOut(AbstractWindow* context)
	{
		RequestRedraw();
	}

	void ComboBox::InitializeComboBox()
	{
		std::vector<GLfloat> inner_verts;
		std::vector<GLfloat> outer_verts;

		if (AbstractWindow::theme->menu().shaded) {
			GenerateRoundedVertices(Vertical,
					AbstractWindow::theme->menu().shadetop,
					AbstractWindow::theme->menu().shadedown,
					&inner_verts,
					&outer_verts);
		} else {
			GenerateRoundedVertices(&inner_verts, &outer_verts);
		}

		buffer_.generate();

		glGenVertexArrays(2, vaos_);

		glBindVertexArray(vaos_[0]);
		buffer_.bind(0);
		buffer_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);

		glEnableVertexAttribArray(
				AttributeCoord);
		glVertexAttribPointer(AttributeCoord,
				3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(vaos_[1]);
		buffer_.bind(1);
		buffer_.set_data(sizeof(GLfloat) * outer_verts.size(), &outer_verts[0]);

		glEnableVertexAttribArray (AttributeCoord);
		glVertexAttribPointer (AttributeCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);
		GLArrayBuffer::reset();
	}

	void ComboBox::OnPopupListDestroyed(AbstractFrame* frame)
	{
		//assert(frame == popup_);
		//popup_->destroyed().disconnectOne(this, &ComboBox::OnPopupListDestroyed);
		//popup_ = 0;
		SetRoundType(RoundAll);
	}

}

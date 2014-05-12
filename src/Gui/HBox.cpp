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
#endif  // __UNIX__

#include <boost/smart_ptr.hpp>
#include <BlendInt/Gui/HBox.hpp>

namespace BlendInt {

	HBox::HBox (int align, int space)
	: AbstractDequeContainer(), m_alignment(align), m_space(space)
	{
		set_size (200, 200);
	}

	HBox::~HBox ()
	{
	}

	bool HBox::Add (AbstractWidget* widget)
	{
		bool ret = false;

		if(PushBackSubWidget(widget)) {

			FillSubWidgetsInHBox(position(), size(), margin(), m_alignment, m_space);

			ret = true;
		}

		return ret;
	}

	bool HBox::Remove (AbstractWidget* widget)
	{
		bool ret = false;

		if(RemoveSubWidget(widget)) {

			FillSubWidgetsInHBox(position(), size(), margin(), m_alignment, m_space);

			ret = true;

		}

		return ret;
	}

	void HBox::SetAlignment (int align)
	{
		if(m_alignment == align) return;

		m_alignment = align;
		FillSubWidgetsInHBox(position(), size(), margin(), align, m_space);
	}

	void HBox::SetSpace (int space)
	{
		if(m_space == space) return;

		m_space = space;
		FillSubWidgetsInHBox(position(), size(), margin(), m_alignment, m_space);
	}

	Size BlendInt::HBox::GetPreferredSize () const
	{
		Size preferred_size;

		if(sub_widget_size() == 0) {

			preferred_size.set_width(200);
			preferred_size.set_height(200);

		} else {

			AbstractWidget* widget = 0;
			Size tmp_size;

			preferred_size.set_width(-m_space);
			for(WidgetDeque::iterator it = sub_widgets()->begin(); it != sub_widgets()->end(); it++)
			{
				widget = *it;

				if(widget->visiable()) {
					tmp_size = widget->GetPreferredSize();

					preferred_size.add_width(tmp_size.width() + m_space);
					preferred_size.set_height(std::max(preferred_size.height(), tmp_size.height()));
				}
			}

			preferred_size.add_width(margin().left() + margin().right());
			preferred_size.add_height(margin().top() + margin().bottom());
		}

		return preferred_size;
	}

	bool HBox::UpdateTest (const UpdateRequest& request)
	{
		if(request.source() == Predefined) {

			switch (request.type()) {

				case SubWidgetSize:
					return false;	// DO not allow sub widget geometry reset outside

				case SubWidgetPosition:
					return false;

				default:
					return AbstractDequeContainer::UpdateTest(request);

			}

		} else {
			return false;
		}
	}

	void HBox::Update (const UpdateRequest& request)
	{
		if(request.source() == Predefined) {

			switch (request.type()) {

				case FormPosition: {
					const Point* new_pos = static_cast<const Point*>(request.data());
					int x = new_pos->x() - position().x();
					int y = new_pos->y() - position().y();
					MoveSubWidgets(x, y);
					break;
				}

				case FormSize: {
					const Size* size_p = static_cast<const Size*>(request.data());
					FillSubWidgetsInHBox(position(), *size_p, margin(), m_alignment, m_space);
					break;
				}

				case ContainerMargin: {
					const Margin* margin_p = static_cast<const Margin*>(request.data());
					FillSubWidgetsInHBox(position(), size(), *margin_p, m_alignment, m_space);
					break;
				}

				case WidgetRefresh: {
					Refresh();
					break;
				}

				default: {
					break;
				}
			}

		}
	}

	ResponseType HBox::Draw (const RedrawEvent& event)
	{
		return IgnoreAndContinue;
	}

	ResponseType HBox::CursorEnterEvent (bool entered)
	{
		return IgnoreAndContinue;
	}

	ResponseType HBox::KeyPressEvent (const KeyEvent& event)
	{
		return IgnoreAndContinue;
	}

	ResponseType HBox::ContextMenuPressEvent (
	        const ContextMenuEvent& event)
	{
		return IgnoreAndContinue;
	}

	ResponseType HBox::ContextMenuReleaseEvent (
	        const ContextMenuEvent& event)
	{
		return IgnoreAndContinue;
	}

	ResponseType HBox::MousePressEvent (const MouseEvent& event)
	{
		return IgnoreAndContinue;
	}

	ResponseType HBox::MouseReleaseEvent (const MouseEvent& event)
	{
		return IgnoreAndContinue;
	}

	ResponseType HBox::MouseMoveEvent (const MouseEvent& event)
	{
		return IgnoreAndContinue;
	}

	void HBox::FillSubWidgetsInHBox (const Point& out_pos, const Size& out_size, const Margin& margin,
	        int alignment, int space)
	{
		int x = out_pos.x() + margin.left();
		int y = out_pos.y() + margin.bottom();
		unsigned int width = out_size.width() - margin.left() - margin.right();
		unsigned int height = out_size.height() - margin.top() - margin.bottom();

		FillSubWidgetsProportionally(x, y, width, height, alignment, space);
	}

	void HBox::FillSubWidgetsInHBox (const Point& pos, const Size& size, int alignment, int space)
	{
		FillSubWidgetsProportionally(pos.x(), pos.y(), size.width(), size.height(), alignment, space);
	}

	void HBox::FillSubWidgetsProportionally (int x, int y, unsigned int width,
					unsigned int height, int alignment, int space)
	{
		boost::scoped_ptr<std::deque<unsigned int> > expandable_preferred_widths(new std::deque<unsigned int>);
		boost::scoped_ptr<std::deque<unsigned int> > unexpandable_preferred_widths(new std::deque<unsigned int>);
		boost::scoped_ptr<std::deque<unsigned int> > unexpandable_preferred_heights(new std::deque<unsigned int>);

		unsigned int expandable_preferred_width_sum = 0;	// the width sum of the expandable widgets' size
		unsigned int unexpandable_preferred_width_sum = 0;	// the width sum of the unexpandable widgets' size

		AbstractWidget* widget = 0;
		Size tmp_size;
		for(WidgetDeque::iterator it = sub_widgets()->begin(); it != sub_widgets()->end(); it++)
		{
			widget = *it;
			if (widget->visiable()) {
				tmp_size = widget->GetPreferredSize();

				if(widget->expand_x()) {
					expandable_preferred_width_sum += tmp_size.width();
					expandable_preferred_widths->push_back(tmp_size.width());
				} else {
					unexpandable_preferred_width_sum += tmp_size.width();
					unexpandable_preferred_widths->push_back(tmp_size.width());
				}

				if(!widget->expand_y()) {
					unexpandable_preferred_heights->push_back(tmp_size.height());
				}

			}
		}

		if((expandable_preferred_widths->size() + unexpandable_preferred_widths->size()) == 0) return;	// do nothing if all sub widgets are invisible

		unsigned int total_space = ((expandable_preferred_widths->size() + unexpandable_preferred_widths->size()) - 1) * space;

		unsigned int total_preferred_width = expandable_preferred_width_sum
						+ unexpandable_preferred_width_sum
						+ total_space;

		if (total_preferred_width == width) {
			DistributeWithPreferredWidth(x, space,
							expandable_preferred_widths.get(),
							unexpandable_preferred_widths.get());
		} else if (total_preferred_width < width) {
			DistributeWithLargeWidth(x, width, space, expandable_preferred_widths.get(),
							expandable_preferred_width_sum, unexpandable_preferred_widths.get(),
							unexpandable_preferred_width_sum);
		} else {
			DistributeWithSmallWidth(x, width, space, expandable_preferred_widths.get(),
							expandable_preferred_width_sum, unexpandable_preferred_widths.get(),
							unexpandable_preferred_width_sum);
		}

		Align(y, height, alignment, unexpandable_preferred_heights.get());
	}

	void HBox::DistributeWithPreferredWidth (int x, int space,
					const std::deque<unsigned int>* expandable_preferred_widths,
					const std::deque<unsigned int>* unexpandable_preferred_widths)
	{
		std::deque<unsigned int>::const_iterator exp_it = expandable_preferred_widths->begin();
		std::deque<unsigned int>::const_iterator unexp_it = unexpandable_preferred_widths->begin();

		WidgetDeque::iterator widget_it = sub_widgets()->begin();
		AbstractWidget* widget = 0;

		while (widget_it != sub_widgets()->end()) {

			widget = *widget_it;

			if(widget->visiable()) {

				if(widget->expand_x()) {
					ResizeSubWidget(widget, *exp_it, widget->size().height());
					SetSubWidgetPosition(widget, x, widget->position().y());
					exp_it++;
				} else {
					ResizeSubWidget(widget, *unexp_it, widget->size().height());
					SetSubWidgetPosition(widget, x, widget->position().y());
					unexp_it++;
				}

				x = x + widget->size().width() + space;
			}

			widget_it++;
		}
	}

	void HBox::DistributeWithSmallWidth (int x,
					unsigned int width,
					int space,
					const std::deque<unsigned int>* expandable_preferred_widths,
					unsigned int expandable_prefer_sum,
					const std::deque<unsigned int>* unexpandable_preferred_widths,
					unsigned int unexpandable_prefer_sum)
	{
		int widgets_width = width - (expandable_preferred_widths->size() + unexpandable_preferred_widths->size() - 1) * space;

		if(widgets_width <= 0) {
			for(WidgetDeque::iterator it = sub_widgets()->begin(); it != sub_widgets()->end(); it++)
			{
				(*it)->Resize(0, (*it)->size().height());
			}
			return;
		}

		unsigned int reference_width;
		std::deque<unsigned int>::const_iterator exp_it = expandable_preferred_widths->begin();
		std::deque<unsigned int>::const_iterator unexp_it = unexpandable_preferred_widths->begin();
		WidgetDeque::iterator it = sub_widgets()->begin();
		AbstractWidget* widget = 0;

		if(widgets_width <= unexpandable_prefer_sum) {
			reference_width = widgets_width;

			while (it != sub_widgets()->end()) {

				widget = (*it);

				if(widget->visiable()) {

					if (widget->expand_x()) {
						ResizeSubWidget(widget, 0, widget->size().height());
						SetSubWidgetPosition(widget, x, widget->position().y());
						exp_it++;
					} else {
						ResizeSubWidget(widget,
										reference_width * (*unexp_it)
														/ unexpandable_prefer_sum,
										widget->size().height());
						SetSubWidgetPosition(widget, x, widget->position().y());
						unexp_it++;
					}

					x = x + widget->size().width() + space;
				}

				it++;
			}

		} else {
			reference_width = widgets_width - unexpandable_prefer_sum;

			while (it != sub_widgets()->end()) {

				widget = (*it);

				if(widget->visiable()) {

					if (widget->expand_x()) {
						ResizeSubWidget(widget,
										reference_width * (*exp_it)
														/ expandable_prefer_sum,
										widget->size().height());
						SetSubWidgetPosition(widget, x, widget->position().y());
						exp_it++;
					} else {
						ResizeSubWidget(widget, (*unexp_it), widget->size().height());
						SetSubWidgetPosition(widget, x, widget->position().y());
						unexp_it++;
					}

					x = x + widget->size().width() + space;
				}

				it++;
			}

		}
	}

	void HBox::DistributeWithLargeWidth (int x,
					unsigned int width,
					int space,
					const std::deque<unsigned int>* expandable_preferred_widths,
					unsigned int expandable_prefer_sum,
					const std::deque<unsigned int>* unexpandable_preferred_widths,
					unsigned int unexpandable_prefer_sum)
	{
		unsigned int widgets_width = width - (expandable_preferred_widths->size() + unexpandable_preferred_widths->size() - 1) * space;

		unsigned int expandable_width = widgets_width - unexpandable_prefer_sum;

		std::deque<unsigned int>::const_iterator exp_it = expandable_preferred_widths->begin();
		std::deque<unsigned int>::const_iterator unexp_it = unexpandable_preferred_widths->begin();

		WidgetDeque::iterator it = sub_widgets()->begin();

		AbstractWidget* widget = 0;
		while (it != sub_widgets()->end()) {

			widget = (*it);

			if(widget->visiable()) {

				if (widget->expand_x()) {
					ResizeSubWidget(widget,
									expandable_width * (*exp_it)
													/ expandable_prefer_sum,
									widget->size().height());
					SetSubWidgetPosition(widget, x, widget->position().y());
					exp_it++;
				} else {
					ResizeSubWidget(widget, (*unexp_it), widget->size().height());
					SetSubWidgetPosition(widget, x, widget->position().y());
					unexp_it++;
				}

				x = x + widget->size().width() + space;
			}

			it++;
		}
	}

	void HBox::Align (int y, unsigned int height, int alignment, const std::deque<unsigned int>* unexpandable_preferred_heights)
	{
		std::deque<unsigned int>::const_iterator unexp_it =
		        unexpandable_preferred_heights->begin();
		WidgetDeque::iterator it;
		AbstractWidget* widget = 0;
		for (it = sub_widgets()->begin(); it != sub_widgets()->end(); it++) {
			widget = *it;

			if (widget->expand_y()) {

				ResizeSubWidget(widget, widget->size().width(), height);
				SetSubWidgetPosition(widget, widget->position().x(), y);

			} else {

				if ((*unexp_it) >= height) {
					ResizeSubWidget(widget, widget->size().width(), height);
					SetSubWidgetPosition(widget, widget->position().x(), y);
				} else {

					ResizeSubWidget(widget, widget->size().width(),
					        (*unexp_it));

					if (alignment & AlignTop) {
						SetSubWidgetPosition(widget, widget->position().x(),
						        y + (height - widget->size().height()));
					} else if (alignment & AlignBottom) {
						SetSubWidgetPosition(widget, widget->position().x(), y);
					} else if (alignment & AlignHorizontalCenter) {
						SetSubWidgetPosition(widget, widget->position().x(),
						        y + (height - widget->size().height()) / 2);
					}
				}

				unexp_it++;
			}
		}
	}

}

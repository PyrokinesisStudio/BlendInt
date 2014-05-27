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

#ifndef _BLENDINT_GUI_ABSTRACTSINGLECONTAINER_HPP_
#define _BLENDINT_GUI_ABSTRACTSINGLECONTAINER_HPP_

#include <BlendInt/Gui/AbstractContainer.hpp>

namespace BlendInt {

	/**
	 * @brief A container hold only 1 sub widget
	 */
	class AbstractSingleContainer: public AbstractContainer
	{
		DISALLOW_COPY_AND_ASSIGN(AbstractSingleContainer);

	public:

		AbstractSingleContainer ();

		virtual ~AbstractSingleContainer ();

		virtual bool IsExpandX () const;

		virtual bool IsExpandY () const;

		virtual Size GetPreferredSize () const;

	protected:

		/**
		 * @brief Fill the sub widget in this container
		 */
		void FillSubWidget (const Point& pos, const Size& out_size, const Margin& margin);

		void FillSubWidget (const Point& pos, const Size& size);

		void FillSubWidget (int x, int y, unsigned w, unsigned h);

		bool SetSubWidget (AbstractWidget* widget);

		virtual bool RemoveSubWidget (AbstractWidget* widget);

		virtual IteratorPtr CreateIterator (const DeviceEvent& event);

		virtual ResponseType FocusEvent (bool focus);

		void MoveSubWidget (int offset_x, int offset_y);

		AbstractWidget* sub_widget () const
		{
			return m_sub_widget;
		}

	private:

		void OnSubWidgetDestroyed (AbstractWidget* widget);

		AbstractWidget* m_sub_widget;

	};
}

#endif /* _BLENDINT_GUI_ABSTRACTSINGLECONTAINER_HPP_ */

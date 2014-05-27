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

#ifndef _BLENDINT_MARGIN_HPP_
#define _BLENDINT_MARGIN_HPP_

/**
 * @brief Margin
 *
 * used for box model of a widget
 */
namespace BlendInt {

	class Margin
	{
	public:

		Margin ()
		: m_left(5), m_right(5), m_top(5), m_bottom(5)
		{}

		Margin (int all)
		: m_left(all), m_right(all), m_top(all), m_bottom(all)
		{}

		Margin (int left, int right, int top, int bottom)
		: m_left(left), m_right(right), m_top(top), m_bottom(bottom)
		{}

		void set_value (int left, int right, int top, int bottom)
		{
			m_left = left;
			m_right = right;
			m_top = top;
			m_bottom = bottom;
		}

		int left () const {return m_left;}

		void set_left (int left) {m_left = left;}

		int right () const {return m_right;}

		void set_right (int right) {m_right = right;}

		int top () const {return m_top;}

		void set_top (int top) {m_top = top;}

		int bottom () const {return m_bottom;}

		void set_bottom (int bottom) {m_bottom = bottom;}

		int hsum () const
		{
			return m_left + m_right;
		}

		int vsum () const
		{
			return m_top + m_bottom;
		}

		bool equal (int left, int right, int top, int bottom)
		{
			return (m_left == left &&
					m_right == right &&
					m_top == top &&
					m_bottom == bottom);
		}

		bool equal (const Margin& margin)
		{
			return (m_left == margin.left() &&
					m_right == margin.right() &&
					m_top == margin.top() &&
					m_bottom == margin.bottom());
		}

	private:

		int m_left;
		int m_right;
		int m_top;
		int m_bottom;
	};
}

#endif /* _BIL_MARGIN_HPP_ */

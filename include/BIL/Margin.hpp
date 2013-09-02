/*
 * This file is part of BIL (Blender Interface Library).
 *
 * BIL (Blender Interface Library) is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * BIL (Blender Interface Library) is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BIL.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Contributor(s): Freeman Zhang <zhanggyb@gmail.com>
 */

#ifndef _BIL_MARGIN_HPP_
#define _BIL_MARGIN_HPP_

/**
 * @brief Margin
 *
 * used for box model of a widget
 */
namespace BIL {

	class Margin
	{
	public:

		Margin ()
		: left_(0), right_(0), top_(0), bottom_(0)
		{}

		Margin (int all)
		: left_(all), right_(all), top_(all), bottom_(all)
		{}

		Margin (int left, int right, int top, int bottom)
		: left_(left), right_(right), top_(top), bottom_(bottom)
		{}

		int left () const {return left_;}

		void set_left (int left) {left_ = left;}

		int right () const {return right_;}

		void set_right (int right) {right_ = right;}

		int top () const {return top_;}

		void set_top (int top) {top_ = top;}

		int bottom () const {return bottom_;}

		void set_bottom (int bottom) {bottom_ = bottom;}

	private:

		int left_;
		int right_;
		int top_;
		int bottom_;
	};
}

#endif /* _BIL_MARGIN_HPP_ */
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

#ifndef _BLENDINT_GUI_FORMBASE_HPP_
#define _BLENDINT_GUI_FORMBASE_HPP_

#include <glm/mat4x4.hpp>

#include <BlendInt/Gui/AbstractForm.hpp>

namespace BlendInt {

	enum FormRequestType {
		FormSize,
		FormRoundType,
		FormRoundRadius
	};

	/**
	 * @brief a simple form class inherit from AbstractForm and provide Resize function
	 */
	class AbstractResizableForm: public AbstractForm
	{
	public:

		AbstractResizableForm ()
		: AbstractForm()
		{}

		virtual ~AbstractResizableForm () {}

		void Resize (int width, int height);

		void Resize (const Size& size);

		virtual void Draw (const glm::mat4& mvp) = 0;

	protected:

		virtual void UpdateGeometry (const UpdateRequest& request) = 0;

	};

}

#endif	// _BLENDINT_GUI_FORMBASE_HPP_

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

#pragma once

#include <deque>

#include <blendint/opengl/gl-buffer.hpp>

#include <blendint/gui/abstract-shadow.hpp>

namespace BlendInt {

/**
 * @brief Soft shadow for frames
 *
 * @ingroup blendint_gui_frames
 */
class FrameShadow: public AbstractShadow
{

public:

  FrameShadow (const Size& size = Size(100, 100),
               int round_type = RoundNone,
               float round_radius = 5.f);

  virtual ~FrameShadow ();

  virtual void Draw (int x,
                     int y,
                     const float* color_ptr = Color(Palette::Black).data(),
                     short gamma = 0,
                     float rotate = 0.f,
                     float scale_x = 1.f,
                     float scale_y = 1.f) const;

protected:

  virtual void PerformSizeUpdate (int width, int height);

  virtual void PerformRoundTypeUpdate (int type);

  virtual void PerformRoundRadiusUpdate (float radius);

private:

  void InitializeFrameShadowOnce ();

  GLuint vao_;

  GLBuffer<ARRAY_BUFFER> vertex_buffer_;

  GLBuffer<ELEMENT_ARRAY_BUFFER> element_buffer_;

};

}

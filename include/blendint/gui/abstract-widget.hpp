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

#pragma once

#include <blendint/gui/abstract-view.hpp>

namespace BlendInt {

class GLTexture2D;
// forward declare

/**
 * @brief A Normal widget
 *
 * @ingroup blendint_gui_widgets
 */
class AbstractWidget: public AbstractView
{
DISALLOW_COPY_AND_ASSIGN(AbstractWidget);

public:

  AbstractWidget ();

  AbstractWidget (int width, int height);

  virtual ~AbstractWidget ();

  CppEvent::EventRef<AbstractWidget*> destroyed ()
  {
    return destroyed_;
  }

protected:

  virtual bool PreDraw (AbstractWindow* context);

  // virtual Response Draw (Profile& profile);

  virtual void PostDraw (AbstractWindow* context);

  virtual void PerformFocusOn (AbstractWindow* context);

  virtual void PerformFocusOff (AbstractWindow* context);

  virtual void PerformHoverIn (AbstractWindow* context);

  virtual void PerformHoverOut (AbstractWindow* context);

  virtual Response PerformKeyPress (AbstractWindow* context);

  virtual Response PerformContextMenuPress (AbstractWindow* context);

  virtual Response PerformContextMenuRelease (AbstractWindow* context);

  virtual Response PerformMousePress (AbstractWindow* context);

  virtual Response PerformMouseRelease (AbstractWindow* context);

  virtual Response PerformMouseMove (AbstractWindow* context);

  static bool RenderSubWidgetsToTexture (AbstractWidget* widget,
                                         AbstractWindow* context,
                                         GLTexture2D* texture);

private:

  CppEvent::Event<AbstractWidget*> destroyed_;

};

}

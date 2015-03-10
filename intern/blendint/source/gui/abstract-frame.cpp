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

#include <stdexcept>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <core/rect.hpp>

#include <opengl/opengl.hpp>
#include <opengl/gl-framebuffer.hpp>

#include <gui/abstract-frame.hpp>
#include <gui/abstract-window.hpp>

namespace BlendInt {

  glm::mat4 AbstractFrame::kViewMatrix = glm::lookAt(glm::vec3(0.f, 0.f, 1.f),
                                                     glm::vec3(0.f, 0.f, 0.f),
                                                     glm::vec3(0.f, 1.f, 0.f));

  AbstractFrame::AbstractFrame (int frame_flag)
  : AbstractView(), frame_flag_(frame_flag)
  {
    destroyed_.reset(new Cpp::Event<AbstractFrame*>);
  }

  AbstractFrame::AbstractFrame (int width, int height, int frame_flag)
  : AbstractView(width, height), frame_flag_(frame_flag)
  {
    destroyed_.reset(new Cpp::Event<AbstractFrame*>);
  }

  AbstractFrame::~AbstractFrame ()
  {
    if (subs_count() > 0) {
      ClearSubViews();
    } else {
      assert(subs_count_ == 0);
      assert(first_subview_ == 0);
      assert(last_subview_ == 0);
    }

    if (superview_) superview_->RemoveSubView(this);

    destroyed_->fire(this);
  }

  Point AbstractFrame::GetAbsolutePosition (const AbstractWidget* widget)
  {
    if (widget == nullptr)
      throw std::invalid_argument("Argument cannot be a nullptr!");

    Point pos = widget->position();
    AbstractFrame* frame = 0;
    AbstractView* p = widget->superview();
    while (p) {

      frame = dynamic_cast<AbstractFrame*>(p);
      if (frame) break;

      pos = pos + p->position() + p->GetOffset();
      p = p->superview();

    }

    if (frame == 0)
      throw std::domain_error("The widget is not added in the context");

    if (frame != this)
      throw std::out_of_range("Widget is not contained in this frame!");

    pos = pos + position() + GetOffset();
    return pos;
  }

  Point AbstractFrame::GetRelativePosition (const AbstractWidget* widget)
  {
    if (widget == nullptr)
      throw std::invalid_argument("Argument cannot be a nullptr!");

    Point pos = widget->position();
    AbstractFrame* frame = 0;
    AbstractView* p = widget->superview();

    while (p) {

      frame = dynamic_cast<AbstractFrame*>(p);
      if (frame) break;

      pos = pos + p->position() + p->GetOffset();
      p = p->superview();

    }

    if (frame == 0)
      throw std::domain_error("The widget is not added in the context");

    if (frame != this)
      throw std::out_of_range("Widget is not contained in this frame!");

    return pos;
  }

  bool AbstractFrame::EnableViewBuffer ()
  {
    if (!view_buffer_) {

#ifdef DEBUG
      if ((size().width() == 0) || (size().height() == 0)) {
        DBG_PRINT_MSG("WARNING: %s", "size invalid");
      }
#endif

      view_buffer_.reset(new ViewBuffer(size().width(), size().height()));

      // TODO: check view buffer and return false if it's not valid
    }

    return true;
  }

  void AbstractFrame::DisableViewBuffer ()
  {
    view_buffer_.destroy();
  }

  AbstractFrame* AbstractFrame::GetFrame (AbstractView* widget)
  {
    AbstractView* container = widget->superview();
    AbstractFrame* frame = 0;

    if (container == 0) {
      return dynamic_cast<AbstractFrame*>(widget);
    } else {
      while (container) {
        frame = dynamic_cast<AbstractFrame*>(container);
        if (frame) break;
        container = container->superview();
      }
    }

    return frame;
  }

  Response AbstractFrame::PerformContextMenuPress (AbstractWindow* context)
  {
    return subs_count() ? Ignore : Finish;
  }

  Response AbstractFrame::PerformContextMenuRelease (AbstractWindow* context)
  {
    return subs_count() ? Ignore : Finish;
  }

  AbstractWidget* AbstractFrame::DispatchMouseHover (AbstractWidget* orig,
                                                     AbstractWindow* context)
  {
    context->register_active_frame(this);

    // find the new top hovered widget
    if (orig != nullptr) {

      if (orig->superview() == 0) { // the widget is just removed from this frame
        return FindAndDispatchTopHoveredWidget(context);
      }

      if (orig->superview() == this) {
        return RecheckAndDispatchTopHoveredWidget(orig, context);
      }

      Rect rect;
      try {
        rect.set_position(
            GetAbsolutePosition(
                dynamic_cast<AbstractWidget*>(orig->superview())));
        rect.set_size(orig->superview()->size());
        return RecheckAndDispatchTopHoveredWidget(rect, orig, context);
      }

      catch (const std::bad_cast& e) {
        DBG_PRINT_MSG("Error: %s", e.what());
        return FindAndDispatchTopHoveredWidget(context);
      }

      catch (const std::invalid_argument& e) {
        DBG_PRINT_MSG("Error: %s", e.what());
        return FindAndDispatchTopHoveredWidget(context);
      }

      catch (const std::domain_error& e) {
        DBG_PRINT_MSG("Error: %s", e.what());
        return FindAndDispatchTopHoveredWidget(context);
      }

      catch (const std::out_of_range& e) {
        DBG_PRINT_MSG("Error: %s", e.what());
        return FindAndDispatchTopHoveredWidget(context);
      }

      catch (...) {
        return FindAndDispatchTopHoveredWidget(context);
      }

    } else {

      return FindAndDispatchTopHoveredWidget(context);

    }
  }

  AbstractWidget* AbstractFrame::RecheckAndDispatchTopHoveredWidget (AbstractWidget* orig,
                                                                     AbstractWindow* context)
  {
    assert(orig->superview() == this);

    AbstractWidget* result = orig;
    Rect rect;

    rect.set_position(position());
    rect.set_size(size());

    bool cursor_in_frame = rect.contains(context->GetGlobalCursorPosition());

    if (cursor_in_frame) {

      Point offset = GetOffset();
      context->set_local_cursor_position(
          context->GetGlobalCursorPosition().x() - rect.x() - offset.x(),
          context->GetGlobalCursorPosition().y() - rect.y() - offset.y());

      if (result->Contain(context->local_cursor_position())) {
        result = RecursiveDispatchHoverEvent(result, context);
      } else {
        dispatch_mouse_hover_out(result, context);
        result = FindAndDispatchTopHoveredWidget(context);
      }

    } else {

      dispatch_mouse_hover_out(result, context);
      result = 0;

    }

    return result;
  }

  AbstractWidget* AbstractFrame::RecheckAndDispatchTopHoveredWidget (Rect& rect,
                                                                     AbstractWidget* orig,
                                                                     AbstractWindow* context)
  {
    assert(orig);
    assert(orig->superview() && orig->superview() != this);

    AbstractWidget* result = orig;
    AbstractView* super_view = result->superview();
    AbstractWidget* super_widget = dynamic_cast<AbstractWidget*>(super_view);
    Point offset;

    bool cursor_in_superview = rect.contains(
        context->GetGlobalCursorPosition());

    if (cursor_in_superview) {

      offset = super_view->GetOffset();
      context->set_local_cursor_position(
          context->GetGlobalCursorPosition().x() - rect.x() - offset.x(),
          context->GetGlobalCursorPosition().y() - rect.y() - offset.y());

      if (result->Contain(context->local_cursor_position())) {
        result = RecursiveDispatchHoverEvent(result, context);
      } else {

        dispatch_mouse_hover_out(result, context);

        // find which contianer contains cursor position
        while (super_view) {

          if (super_view == this) {
            super_view = 0;
            break;
          }

          offset = super_view->GetOffset();
          context->set_local_cursor_position(
              context->local_cursor_position().x() + super_view->position().x()
                  + offset.x(),
              context->local_cursor_position().y() + super_view->position().y()
                  + offset.y());

          if (super_view->Contain(context->local_cursor_position())) break;

          super_view = super_view->superview();
        }

        result = dynamic_cast<AbstractWidget*>(super_view);

        if (result) {
          result = RecursiveDispatchHoverEvent(result, context);
        }

      }

    } else {

      dispatch_mouse_hover_out(result, context);

      // find which contianer contains cursor position
      super_view = super_view->superview();
      while (super_view != nullptr) {

        if (super_view == this) {
          super_view = nullptr;
          break;
        }

        super_widget = dynamic_cast<AbstractWidget*>(super_view);
        if (super_widget) {
          rect.set_position(GetAbsolutePosition(super_widget));
          rect.set_size(super_widget->size());
        } else {
          assert(super_view == this);
          rect.set_position(position());
          rect.set_size(size());
        }

        offset = super_view->GetOffset();
        context->set_local_cursor_position(
            context->GetGlobalCursorPosition().x() - rect.x() - offset.x(),
            context->GetGlobalCursorPosition().y() - rect.y() - offset.y());

        if (rect.contains(context->GetGlobalCursorPosition())) break;

        super_view = super_view->superview();
      }

      result = dynamic_cast<AbstractWidget*>(super_view);
      if (result) {

        AbstractWidget* tmp = 0;
        for (AbstractView* p = super_widget->last_subview(); p;
            p = p->previous_view()) {

          tmp = dynamic_cast<AbstractWidget*>(p);
          if (tmp) {
            if (p->visiable() && p->Contain(context->local_cursor_position())) {
              result = tmp;

              dispatch_mouse_hover_in(result, context);
              result = RecursiveDispatchHoverEvent(result, context);

              break;
            }
          } else {
            break;
          }
        }
      }

    }

    return result;
  }

  AbstractWidget* AbstractFrame::FindAndDispatchTopHoveredWidget (AbstractWindow* context)
  {
    AbstractWidget* result = 0;
    Point offset;

    offset = GetOffset();
    context->set_local_cursor_position(
        context->GetGlobalCursorPosition().x() - position().x() - offset.x(),
        context->GetGlobalCursorPosition().y() - position().y() - offset.y());

    for (AbstractView* p = last_subview(); p; p = p->previous_view()) {

      result = dynamic_cast<AbstractWidget*>(p);
      if (result) {
        if (p->visiable() && p->Contain(context->local_cursor_position())) {
          dispatch_mouse_hover_in(result, context);
          break;
        }
      } else {
        break;
      }

    }

    if (result) {
      result = RecursiveDispatchHoverEvent(result, context);
    }

    return result;
  }

  void AbstractFrame::ClearHoverWidgets (AbstractView* hovered_widget)
  {
#ifdef DEBUG
    assert(hovered_widget);
#endif

    while (hovered_widget && (hovered_widget != this)) {
      hovered_widget = hovered_widget->superview();
    }
  }

  void AbstractFrame::ClearHoverWidgets (AbstractView* hovered_widget,
                                         AbstractWindow* context)
  {
#ifdef DEBUG
    assert(hovered_widget);
#endif

    while (hovered_widget && (hovered_widget != this)) {
      hovered_widget->PerformHoverOut(context);
      hovered_widget = hovered_widget->superview();
    }
  }

  bool AbstractFrame::RenderSubFramesToTexture (AbstractFrame* frame,
                                                AbstractWindow* context,
                                                const glm::mat4& projection,
                                                const glm::mat3& model,
                                                GLTexture2D* texture)
  {
    bool retval = false;

    assert(texture != nullptr);

    GLTexture2D* tex = texture;
    if (!tex->id()) tex->generate();

    tex->bind();
    tex->SetWrapMode(GL_REPEAT, GL_REPEAT);
    tex->SetMinFilter(GL_NEAREST);
    tex->SetMagFilter(GL_NEAREST);
    tex->SetImage(0, GL_RGBA, frame->size().width(), frame->size().height(), 0,
    GL_RGBA,
                  GL_UNSIGNED_BYTE, 0);

    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    GLFramebuffer* fb = new GLFramebuffer;
    fb->generate();
    fb->bind();

    // Set "renderedTexture" as our colour attachement #0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
    GL_TEXTURE_2D,
                           tex->id(), 0);
    //fb->Attach(*tex, GL_COLOR_ATTACHMENT0);

    // Critical: Create a Depth_STENCIL renderbuffer for this off-screen rendering
    GLuint rb;
    glGenRenderbuffers(1, &rb);

    glBindRenderbuffer(GL_RENDERBUFFER, rb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL,
                          frame->size().width(), frame->size().height());
    //Attach depth buffer to FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
    GL_RENDERBUFFER,
                              rb);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
    GL_RENDERBUFFER,
                              rb);

    if (GLFramebuffer::CheckStatus()) {

      AbstractWindow::shaders()->SetWidgetProjectionMatrix(projection);
      AbstractWindow::shaders()->SetWidgetModelMatrix(model);

      // in this off-screen framebuffer, a new stencil buffer was created, reset the stencil count to 0 and restore later
      GLuint original_stencil_count = context->stencil_count_;
      context->stencil_count_ = 0;

      glClearColor(0.f, 0.f, 0.f, 0.f);
      glClearDepth(1.0);
      glClearStencil(0);

      glClear(
      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

      glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
      GL_ONE_MINUS_SRC_ALPHA);
      //glEnable(GL_BLEND);

      glViewport(0, 0, frame->size().width(), frame->size().height());

      // Draw context:
      frame->DrawSubViewsOnce(context);

      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      glViewport(0, 0, context->size().width(), context->size().height());
#ifdef DEBUG
      assert(context->stencil_count_ == 0);
#endif
      context->stencil_count_ = original_stencil_count;

      retval = true;
    }

    fb->reset();
    tex->reset();

    //delete tex; tex = 0;

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glDeleteRenderbuffers(1, &rb);

    delete fb;
    fb = nullptr;

    return retval;
  }

  AbstractWidget* AbstractFrame::RecursiveDispatchHoverEvent (AbstractWidget* widget,
                                                              AbstractWindow* context)
  {
    AbstractWidget* retval = widget;
    AbstractWidget* tmp = 0;

    Point offset = widget->GetOffset();
    context->set_local_cursor_position(
        context->local_cursor_position().x() - widget->position().x()
            - offset.x(),
        context->local_cursor_position().y() - widget->position().y()
            - offset.y());

    for (AbstractView* p = widget->last_subview(); p; p = p->previous_view()) {

      tmp = dynamic_cast<AbstractWidget*>(p);

      if (tmp) {
        if (p->visiable() && p->Contain(context->local_cursor_position())) {
          retval = tmp;
          dispatch_mouse_hover_in(retval, context);
          retval = RecursiveDispatchHoverEvent(retval, context);

          break;
        }
      } else {
        break;
      }

    }

    return retval;
  }

}
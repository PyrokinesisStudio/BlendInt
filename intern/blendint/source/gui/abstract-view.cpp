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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <opengl/opengl.hpp>

#include <gui/abstract-view.hpp>
#include <gui/abstract-window.hpp>

namespace BlendInt {

  bool IsContained (AbstractView* container, AbstractView* widget)
  {
    bool retval = false;

    AbstractView* p = widget->superview();
    while (p) {
      if (p == container) {
        retval = true;
        break;
      }
      p = p->superview();
    }

    return retval;
  }

  float AbstractView::kBorderWidth = 1.f;

  boost::mutex AbstractView::kRefreshMutex;

  const float AbstractView::cornervec[WIDGET_CURVE_RESOLU][2] = { { 0.0, 0.0 },
      { 0.195, 0.02 }, { 0.383, 0.067 }, { 0.55, 0.169 }, { 0.707, 0.293 }, {
          0.831, 0.45 }, { 0.924, 0.617 }, { 0.98, 0.805 }, { 1.0, 1.0 } };

  AbstractView::AbstractView ()
  : Object(),
    managed_(true),
    visible_(true),
    refresh_(false),
    subs_count_(0),
    superview_(0),
    previous_view_(0),
    next_view_(0),
    first_subview_(0),
    last_subview_(0)
  {
  }

  AbstractView::AbstractView (int width, int height)
  : Object(),
    managed_(true),
    visible_(true),
    refresh_(false),
    subs_count_(0),
    superview_(0),
    previous_view_(0),
    next_view_(0),
    first_subview_(0),
    last_subview_(0)
  {
    set_size(std::abs(width), std::abs(height));
  }

  AbstractView::~AbstractView ()
  {
    if (subs_count() > 0) {
      ClearSubViews();
    } else {
      assert(subs_count_ == 0);
      assert(first_subview_ == 0);
      assert(last_subview_ == 0);
    }

    if (superview_) {
      superview_->RemoveSubView(this);
    } else {
      assert(previous_view_ == 0);
      assert(next_view_ == 0);
    }
  }

  Point AbstractView::GetGlobalPosition () const
  {
    Point retval = position_;
    ;

    AbstractView* p = superview_;
    while (p) {
      retval = retval + p->position() + p->GetOffset();
      p = p->superview_;
    }

    return retval;
  }

  Size AbstractView::GetPreferredSize () const
  {
    return Size(200, 200);
  }

  void AbstractView::Resize (int width, int height)
  {
    if (size().width() == width && size().height() == height) return;

    Size new_size(width, height);
    SizeUpdateRequest request(this, this, &new_size);

    if (superview_) {
      if (superview_->SizeUpdateTest(request) && SizeUpdateTest(request)) {
        PerformSizeUpdate(request);
        set_size(width, height);
      }
    } else {
      if (SizeUpdateTest(request)) {
        PerformSizeUpdate(request);
        set_size(width, height);
      }
    }
  }

  void AbstractView::Resize (const Size& size)
  {
    if (AbstractView::size() == size) return;

    SizeUpdateRequest request(this, this, &size);

    if (superview_) {
      if (superview_->SizeUpdateTest(request) && SizeUpdateTest(request)) {
        PerformSizeUpdate(request);
        set_size(size);
      }
    } else {
      if (SizeUpdateTest(request)) {
        PerformSizeUpdate(request);
        set_size(size);
      }
    }
  }

  void AbstractView::MoveTo (int x, int y)
  {
    if (position().x() == x && position().y() == y) return;

    Point new_pos(x, y);
    PositionUpdateRequest request(this, this, &new_pos);

    if (superview_) {
      if (superview_->PositionUpdateTest(request)
          && PositionUpdateTest(request)) {
        PerformPositionUpdate(request);
        set_position(x, y);
      }
    } else {
      if (PositionUpdateTest(request)) {
        PerformPositionUpdate(request);
        set_position(x, y);
      }
    }
  }

  void AbstractView::MoveTo (const Point& pos)
  {
    if (position() == pos) return;

    PositionUpdateRequest request(this, this, &pos);

    if (superview_) {
      if (superview_->PositionUpdateTest(request)
          && PositionUpdateTest(request)) {
        PerformPositionUpdate(request);
        set_position(pos);
      }
    } else {
      if (PositionUpdateTest(request)) {
        PerformPositionUpdate(request);
        set_position(pos);
      }
    }
  }

  void AbstractView::SetVisible (bool visible)
  {
    if (this->visiable() == visible) return;

    VisibilityUpdateRequest request(this, this, &visible);

    if (superview_) {
      if (superview_->VisibilityUpdateTest(request)
          && VisibilityUpdateTest(request)) {
        PerformVisibilityUpdate(request);
        set_visible(visible);
      }
    } else {
      if (VisibilityUpdateTest(request)) {
        PerformVisibilityUpdate(request);
        set_visible(visible);
      }
    }
  }

  bool AbstractView::Contain (const Point& point) const
  {
    if (point.x() < position_.x() || point.y() < position_.y()
        || point.x() > (position_.x() + size_.width())
        || point.y() > (position_.y() + size_.height())) {
      return false;
    }

    return true;
  }

  void AbstractView::RequestRedraw ()
  {
    if (!refresh()) {

      AbstractView* root = this;
      AbstractView* p = superview();
      boost::thread::id id = boost::this_thread::get_id();

      /*
       while(p) {
       DBG_PRINT_MSG("superview name: %s, refresh flag: %s", p->name().c_str(), p->refresh() ? "True":"False");
       p = p->superview();
       }
       p = superview();
       */

      if (id == AbstractWindow::main_thread_id()) {

        set_refresh(true);
        while (p && (!p->refresh()) && (p->visiable())) {
          root = p;
          p->set_refresh(true);
          p = p->superview();
        }

      } else {

        kRefreshMutex.lock();
        set_refresh(true);
        kRefreshMutex.unlock();

        while (p && (!p->refresh()) && (p->visiable())) {
          root = p;
          kRefreshMutex.lock();
          p->set_refresh(true);
          kRefreshMutex.unlock();
          p = p->superview();
        }

      }

      if (root->superview() == 0) {
        AbstractWindow* window = dynamic_cast<AbstractWindow*>(root);
        if (window) {
          window->Synchronize();
        }
      }

    }
  }

  void AbstractView::SetDefaultBorderWidth (float border)
  {
    kBorderWidth = border;
  }

  void AbstractView::MoveToFirst (AbstractView* view)
  {
    if (view->superview_) {

      if (view->superview_->first_subview_ == view) {
        assert(view->previous_view_ == 0);
        return;	// already at first
      }

      view->previous_view_->next_view_ = view->next_view_;
      if (view->next_view_) {
        view->next_view_->previous_view_ = view->previous_view_;
      } else {
        assert(view->superview_->last_subview_ == view);
        view->superview_->last_subview_ = view->previous_view_;
      }

      view->previous_view_ = 0;
      view->next_view_ = view->superview_->first_subview_;
      view->superview_->first_subview_->previous_view_ = view;
      view->superview_->first_subview_ = view;

      view->superview_->RequestRedraw();
    }
  }

  void AbstractView::MoveToLast (AbstractView* view)
  {
    if (view->superview_) {

      if (view->superview_->last_subview_ == view) {
        assert(view->next_view_ == 0);
        return;	// already at last
      }

      view->next_view_->previous_view_ = view->previous_view_;

      if (view->previous_view_) {
        view->previous_view_->next_view_ = view->next_view_;
      } else {
        assert(view->superview_->first_subview_ == view);
        view->superview_->first_subview_ = view->next_view_;
      }

      view->next_view_ = 0;
      view->previous_view_ = view->superview_->last_subview_;
      view->superview_->last_subview_->next_view_ = view;
      view->superview_->last_subview_ = view;

      view->superview_->RequestRedraw();
    }
  }

  void AbstractView::MoveForward (AbstractView* view)
  {
    if (view->superview_) {

      if (view->next_view_) {

        AbstractView* tmp = view->next_view_;

        tmp->previous_view_ = view->previous_view_;
        if (view->previous_view_) {
          view->previous_view_->next_view_ = tmp;
        } else {
          assert(view->superview_->first_subview_ == view);
          view->superview_->first_subview_ = tmp;
        }

        view->previous_view_ = tmp;
        view->next_view_ = tmp->next_view_;
        if (tmp->next_view_) {
          tmp->next_view_->previous_view_ = view;
        }
        tmp->next_view_ = view;

        if (view->next_view_ == 0) {
          assert(view->superview_->last_subview_ == tmp);
          view->superview_->last_subview_ = view;
        }

        if (view->previous_view_) {
          assert(view->previous_view_->next_view_ == view);
        }
        if (view->next_view_) {
          assert(view->next_view_->previous_view_ == view);
        }

        view->superview_->RequestRedraw();

      } else {
        assert(view->superview_->last_subview_ == view);
      }

    }
  }

  void AbstractView::MoveBackward (AbstractView* view)
  {
    if (view->superview_) {

      if (view->previous_view_) {

        AbstractView* tmp = view->previous_view_;

        tmp->next_view_ = view->next_view_;
        if (view->next_view_) {
          view->next_view_->previous_view_ = tmp;
        } else {
          assert(view->superview_->last_subview_ == view);
          view->superview_->last_subview_ = tmp;
        }

        view->next_view_ = tmp;
        view->previous_view_ = tmp->previous_view_;
        if (tmp->previous_view_) {
          tmp->previous_view_->next_view_ = view;
        }
        tmp->previous_view_ = view;

        if (view->previous_view_ == 0) {
          assert(view->superview_->first_subview_ == tmp);
          view->superview_->first_subview_ = view;
        }

        DBG_PRINT_MSG("this: %s", view->name_.c_str());
        if (view->previous_view_) {
          DBG_PRINT_MSG("previous_view: %s",
                        view->previous_view_->name_.c_str());
          assert(view->previous_view_->next_view_ == view);
        }
        if (view->next_view_) {
          DBG_PRINT_MSG("next_view: %s", view->next_view_->name_.c_str());
          assert(view->next_view_->previous_view_ == view);
        }

        view->superview_->RequestRedraw();

      } else {
        assert(view->superview_->first_subview_ == view);
      }

    }
  }

  int AbstractView::GetOutlineVertices (int round_type)
  {
    round_type = round_type & RoundAll;
    int count = 0;

    while (round_type != 0) {
      count += round_type & 0x1;
      round_type = round_type >> 1;
    }

    return 4 - count + count * WIDGET_CURVE_RESOLU;
  }

  void AbstractView::DrawSubViewsOnce (AbstractWindow* context)
  {
    //bool refresh_record = false;

    for (AbstractView* p = first_subview(); p; p = p->next_view()) {
      set_refresh(false);	// allow pass to superview in RequestRedraw()
      if (p->PreDraw(context)) {

        Response response = p->Draw(context);
        kRefreshMutex.lock();
        p->set_refresh(refresh());
        kRefreshMutex.unlock();

        if (response == Ignore) {
          for (AbstractView* sub = p->first_subview(); sub; sub =
              sub->next_view()) {
            DispatchDrawEvent(sub, context);
          }
        }

        p->PostDraw(context);
      }

      //if(refresh()) refresh_record = true;
    }

    //set_refresh(refresh_record);
    if (superview_) {
      kRefreshMutex.lock();
      set_refresh(superview_->refresh());
      kRefreshMutex.unlock();
    } else {
      kRefreshMutex.lock();
      set_refresh(false);
      kRefreshMutex.unlock();
    }
  }

  bool AbstractView::SwapIndex (AbstractView *view1, AbstractView *view2)
  {
    if (view1 == nullptr || view2 == nullptr) return false;
    if (view1 == view2) return false;
    if (view1->superview_ != view2->superview_) return false;
    if (view1->superview_ == nullptr) return false;

    AbstractView* tmp1 = nullptr;
    AbstractView* tmp2 = nullptr;

    if (view1->next_view_ == view2) {	// view1 is just the previous sibling of view2

      assert(view2->previous_view_ == view1);

      tmp1 = view1->previous_view_;
      tmp2 = view2->next_view_;

      view2->previous_view_ = tmp1;
      view1->previous_view_ = view2;

      view2->next_view_ = view1;
      view1->next_view_ = tmp2;

      if (tmp1 != nullptr) {
        tmp1->next_view_ = view2;
      } else {
        view1->superview_->first_subview_ = view2;
      }

      if (tmp2 != nullptr) {
        tmp2->previous_view_ = view1;
      } else {
        view2->superview_->last_subview_ = view2;
      }

    } else if (view1->previous_view_ == view2) {

      assert(view2->next_view_ == view1);

      tmp1 = view2->previous_view_;
      tmp2 = view1->next_view_;

      view1->previous_view_ = tmp1;
      view2->previous_view_ = view1;

      view1->next_view_ = view2;
      view2->next_view_ = tmp2;

      if (tmp1 != nullptr) {
        tmp1->next_view_ = view1;
      } else {
        view2->superview_->first_subview_ = view1;
      }

      if (tmp2 != nullptr) {
        tmp2->previous_view_ = view2;
      } else {
        view1->superview_->last_subview_ = view2;
      }

    } else {

      tmp1 = view1->previous_view_;
      tmp2 = view2->previous_view_;

      view1->previous_view_ = tmp2;
      view2->previous_view_ = tmp1;

      if (tmp1 != nullptr) {
        tmp1->next_view_ = view2;
      } else {
        view1->superview_->first_subview_ = view2;
      }

      if (tmp2 != nullptr) {
        tmp2->next_view_ = view1;
      } else {
        view2->superview_->first_subview_ = view1;
      }

      tmp1 = view1->next_view_;
      tmp2 = view2->next_view_;

      view1->next_view_ = tmp2;
      view2->next_view_ = tmp1;

      if (tmp1 != nullptr) {
        tmp1->previous_view_ = view2;
      } else {
        view1->superview_->last_subview_ = view2;
      }

      if (tmp2 != nullptr) {
        tmp2->previous_view_ = view1;
      } else {
        view2->superview_->last_subview_ = view1;
      }

    }

    return true;
  }

  bool AbstractView::InsertSiblingBefore (AbstractView *src, AbstractView *dst)
  {
    if (src == nullptr || dst == nullptr) return false;
    if (src == dst) return false;

    if (dst->superview_ != nullptr) {

      if (dst->superview_ == src->superview_) {

        if (src->previous_view_ == dst) {	// already is the previous one of src
          return true;
        }

        if (dst->previous_view_) {
          dst->previous_view_->next_view_ = dst->next_view_;
        } else {
          assert(dst->superview_->first_subview_ == dst);
          dst->superview_->first_subview_ = dst->next_view_;
        }

        if (dst->next_view_) {
          dst->next_view_->previous_view_ = dst->previous_view_;
        } else {
          assert(dst->superview_->last_subview_ == dst);
          dst->superview_->last_subview_ = dst->previous_view_;
        }

        AbstractView* tmp = src->previous_view_;

        src->previous_view_ = dst;
        dst->next_view_ = src;
        dst->previous_view_ = tmp;

        if (tmp) {
          tmp->next_view_ = dst;
        } else {
          assert(src->superview_->first_subview_ == src);
          dst->superview_->first_subview_ = dst;
        }

        return true;

      } else {
        dst->superview_->RemoveSubView(dst);
      }

    }

    assert(dst->superview_ == nullptr);
    assert(dst->next_view_ == nullptr);
    assert(dst->previous_view_ == nullptr);

    AbstractView* tmp = src->previous_view_;

    src->previous_view_ = dst;
    dst->next_view_ = src;
    dst->previous_view_ = tmp;
    if (tmp) {
      tmp->next_view_ = dst;
    } else {
      assert(src->superview_->first_subview_ == src);
      src->superview_->first_subview_ = dst;
    }

    dst->superview_ = src->superview_;
    src->superview_->subs_count_++;

    return true;
  }

  bool AbstractView::InsertSiblingAfter (AbstractView *src, AbstractView *dst)
  {
    if (src == nullptr || dst == nullptr) return false;
    if (src == dst) return false;

    if (dst->superview_ != nullptr) {

      if (dst->previous_view_ == src->superview_) {

        if (src->next_view_ == dst) {	// alrady is the next one of src
          return true;
        }

        if (dst->previous_view_) {
          dst->previous_view_->next_view_ = dst->next_view_;
        } else {
          assert(dst->superview_->first_subview_ == dst);
          dst->superview_->first_subview_ = dst->next_view_;
        }

        if (dst->next_view_) {
          dst->next_view_->previous_view_ = dst->previous_view_;
        } else {
          assert(dst->superview_->last_subview_ == dst);
          dst->superview_->last_subview_ = dst->previous_view_;
        }

        AbstractView* tmp = src->next_view_;

        src->next_view_ = dst;
        dst->previous_view_ = src;
        dst->next_view_ = tmp;

        if (tmp) {
          tmp->previous_view_ = dst;
        } else {
          assert(src->superview_->last_subview_ == src);
          dst->superview_->last_subview_ = dst;
        }

        return true;

      } else {
        dst->superview_->RemoveSubView(dst);
      }

    }

    assert(dst->superview_ == nullptr);
    assert(dst->next_view_ == nullptr);
    assert(dst->previous_view_ == nullptr);

    AbstractView* tmp = src->next_view_;

    src->next_view_ = dst;
    dst->previous_view_ = src;
    dst->next_view_ = tmp;
    if (tmp) {
      tmp->previous_view_ = dst;
    } else {
      assert(src->superview_->last_subview_ == src);
      src->superview_->last_subview_ = dst;
    }

    dst->superview_ = src->superview_;
    src->superview_->subs_count_++;

    return true;
  }

  void AbstractView::DispatchDrawEvent (AbstractView* widget,
                                        AbstractWindow* context)
  {
#ifdef DEBUG
    assert(widget != 0);
#endif

    if (widget->PreDraw(context)) {

      Response response = widget->Draw(context);

      kRefreshMutex.lock();
      widget->set_refresh(widget->superview_->refresh());
      kRefreshMutex.unlock();

      if (response == Ignore) {
        for (AbstractView* sub = widget->first_subview(); sub;
            sub = sub->next_view()) {
          DispatchDrawEvent(sub, context);
        }
      }

      widget->PostDraw(context);
    }
  }

  bool AbstractView::SizeUpdateTest (const SizeUpdateRequest& request)
  {
    if (request.source()->superview() == this) {
      return false;
    } else {
      return true;
    }
  }

  bool AbstractView::PositionUpdateTest (const PositionUpdateRequest& request)
  {
    if (request.source()->superview() == this) {
      return false;
    } else {
      return true;
    }
  }

  void AbstractView::PerformSizeUpdate (const SizeUpdateRequest& request)
  {
    if (request.target() == this) {
      set_size(*request.size());
    }

    if (request.source() == this) {
      ReportSizeUpdate(request);
    }
  }

  void AbstractView::PerformPositionUpdate (const PositionUpdateRequest& request)
  {
    if (request.target() == this) {
      set_position(*request.position());
    }

    if (request.source() == this) {
      ReportPositionUpdate(request);
    }
  }

  bool AbstractView::VisibilityUpdateTest (const VisibilityUpdateRequest& request)
  {
    return true;
  }

  void AbstractView::PerformVisibilityUpdate (const VisibilityUpdateRequest& request)
  {
    if (request.target() == this) {
      set_visible(*request.visibility());
    }

    if (request.source() == this) {
      ReportVisibilityRequest(request);
    }
  }

  void AbstractView::ReportSizeUpdate (const SizeUpdateRequest& request)
  {
    if (superview_) {
      superview_->PerformSizeUpdate(request);
    }
  }

  void AbstractView::ReportPositionUpdate (const PositionUpdateRequest& request)
  {
    if (superview_) {
      superview_->PerformPositionUpdate(request);
    }
  }

  void AbstractView::ReportVisibilityRequest (const VisibilityUpdateRequest& request)
  {

    if (superview_) {
      superview_->PerformVisibilityUpdate(request);
    }
  }

  int AbstractView::GetHalfOutlineVertices (int round_type)
  {
    round_type = round_type & (RoundBottomLeft | RoundBottomRight);
    int count = 0;
    while (round_type != 0) {
      count += round_type & 0x1;
      round_type = round_type >> 1;
    }

    return 2 - count + count * WIDGET_CURVE_RESOLU;
  }

  /*

   void AbstractView::GenerateVertices(std::vector<GLfloat>* inner,
   std::vector<GLfloat>* outer)
   {
   if(inner == 0 && outer == 0) return;

   std::vector<GLfloat>* inner_ptr = 0;

   if(inner == 0) {
   inner_ptr = new std::vector<GLfloat>;
   } else {
   inner_ptr = inner;
   }

   float border = default_border_width * AbstractWindow::theme()->pixel();

   float rad = round_radius_ * AbstractWindow::theme()->pixel();
   float radi = rad - border;

   float vec[WIDGET_CURVE_RESOLU][2], veci[WIDGET_CURVE_RESOLU][2];

   float minx = 0.0;
   float miny = 0.0;
   float maxx = size_.width();
   float maxy = size_.height();

   float minxi = minx + border;		// U.pixelsize; // boundbox inner
   float maxxi = maxx - border; 	// U.pixelsize;
   float minyi = miny + border;		// U.pixelsize;
   float maxyi = maxy - border;		// U.pixelsize;

   int minsize = 0;
   int corner = round_type();
   const int hnum = (
   (corner & (RoundTopLeft | RoundTopRight)) == (RoundTopLeft | RoundTopRight)
   ||
   (corner & (RoundBottomRight | RoundBottomLeft)) == (RoundBottomRight | RoundBottomLeft)
   ) ? 1 : 2;
   const int vnum = (
   (corner & (RoundTopLeft | RoundBottomLeft)) == (RoundTopLeft | RoundBottomLeft)
   ||
   (corner & (RoundTopRight | RoundBottomRight)) == (RoundTopRight | RoundBottomRight)
   ) ? 1 : 2;

   int count = 0;
   while (corner != 0) {
   count += corner & 0x1;
   corner = corner >> 1;
   }
   unsigned int outline_vertex_number = 4 - count + count * WIDGET_CURVE_RESOLU;
   corner = round_type();

   minsize = std::min(size_.width() * hnum, size_.height() * vnum);

   if (2.0f * rad > minsize)
   rad = 0.5f * minsize;

   if (2.0f * (radi + border) > minsize)
   radi = 0.5f * minsize - border;	// U.pixelsize;

   // mult
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++) {
   veci[i][0] = radi * cornervec[i][0];
   veci[i][1] = radi * cornervec[i][1];
   vec[i][0] = rad * cornervec[i][0];
   vec[i][1] = rad * cornervec[i][1];
   }

   {	// generate inner vertices
   if(inner_ptr->size() != ((outline_vertex_number + 2) * 3)) {
   inner_ptr->resize((outline_vertex_number + 2) * 3);
   }

   // inner_ptr[0, 0] is the center of a triangle fan
   ((*inner_ptr))[0] = minxi + (maxxi - minxi) / 2.f;
   (*inner_ptr)[1] = minyi + (maxyi - minyi) / 2.f;
   (*inner_ptr)[2] = 0.f;

   count = 1;

   // corner left-bottom
   if (corner & RoundBottomLeft) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   (*inner_ptr)[count * 3] = minxi + veci[i][1];
   (*inner_ptr)[count * 3 + 1] = minyi + radi - veci[i][0];
   (*inner_ptr)[count * 3 + 2] = 0.f;
   }
   } else {
   (*inner_ptr)[count * 3] = minxi;
   (*inner_ptr)[count * 3 + 1] = minyi;
   (*inner_ptr)[count * 3 + 2] = 0.f;
   count++;
   }

   // corner right-bottom
   if (corner & RoundBottomRight) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   (*inner_ptr)[count * 3] = maxxi - radi + veci[i][0];
   (*inner_ptr)[count * 3 + 1] = minyi + veci[i][1];
   (*inner_ptr)[count * 3 + 2] = 0.f;
   }
   } else {
   (*inner_ptr)[count * 3] = maxxi;
   (*inner_ptr)[count * 3 + 1] = minyi;
   (*inner_ptr)[count * 3 + 2] = 0.f;
   count++;
   }

   // corner right-top
   if (corner & RoundTopRight) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   (*inner_ptr)[count * 3] = maxxi - veci[i][1];
   (*inner_ptr)[count * 3 + 1] = maxyi - radi + veci[i][0];
   (*inner_ptr)[count * 3 + 2] = 0.f;
   }
   } else {
   (*inner_ptr)[count * 3] = maxxi;
   (*inner_ptr)[count * 3 + 1] = maxyi;
   (*inner_ptr)[count * 3 + 2] = 0.f;
   count++;
   }

   // corner left-top
   if (corner & RoundTopLeft) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   (*inner_ptr)[count * 3] = minxi + radi - veci[i][0];
   (*inner_ptr)[count * 3 + 1] = maxyi - veci[i][1];
   (*inner_ptr)[count * 3 + 2] = 0.f;
   }

   } else {
   (*inner_ptr)[count * 3] = minxi;
   (*inner_ptr)[count * 3 + 1] = maxyi;
   (*inner_ptr)[count * 3 + 2] = 0.f;
   count++;
   }

   (*inner_ptr)[count * 3] = (*inner_ptr)[3 + 0];
   (*inner_ptr)[count * 3 + 1] = (*inner_ptr)[3 + 1];
   (*inner_ptr)[count * 3 + 2] = 0.f;
   }

   if(outer) {

   if(border > 0.f) {

   std::vector<GLfloat> edge_vertices(outline_vertex_number * 2);

   count = 0;

   // corner left-bottom
   if (corner & RoundBottomLeft) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   edge_vertices[count * 2] = minx + vec[i][1];
   edge_vertices[count * 2 + 1] = miny + rad - vec[i][0];
   }
   } else {
   edge_vertices[count * 2] = minx;
   edge_vertices[count * 2 + 1] = miny;
   count++;
   }

   // corner right-bottom
   if (corner & RoundBottomRight) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   edge_vertices[count * 2] = maxx - rad + vec[i][0];
   edge_vertices[count * 2 + 1] = miny + vec[i][1];
   }
   } else {
   edge_vertices[count * 2] = maxx;
   edge_vertices[count * 2 + 1] = miny;
   count++;
   }

   // m_half = count;

   // corner right-top
   if (corner & RoundTopRight) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   edge_vertices[count * 2] = maxx - vec[i][1];
   edge_vertices[count * 2 + 1] = maxy - rad + vec[i][0];
   }
   } else {
   edge_vertices[count * 2] = maxx;
   edge_vertices[count * 2 + 1] = maxy;
   count++;
   }

   // corner left-top
   if (corner & RoundTopLeft) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   edge_vertices[count * 2] = minx + rad - vec[i][0];
   edge_vertices[count * 2 + 1] = maxy - vec[i][1];
   }
   } else {
   edge_vertices[count * 2] = minx;
   edge_vertices[count * 2 + 1] = maxy;
   count++;
   }

   GenerateTriangleStripVertices(inner_ptr, &edge_vertices, count, outer);

   } else {

   outer->clear();

   }

   }

   if(inner == 0) {
   delete inner_ptr;
   }
   }

   void AbstractView::GenerateVertices(Orientation shadedir, short shadetop,
   short shadedown, std::vector<GLfloat>* inner,
   std::vector<GLfloat>* outer)
   {
   if(inner == 0 && outer == 0) return;

   std::vector<GLfloat>* inner_ptr = 0;

   if(inner == 0) {
   inner_ptr = new std::vector<GLfloat>;
   } else {
   inner_ptr = inner;
   }

   float border = default_border_width * AbstractWindow::theme()->pixel();

   float rad = round_radius_ * AbstractWindow::theme()->pixel();
   float radi = rad - border;

   float vec[WIDGET_CURVE_RESOLU][2], veci[WIDGET_CURVE_RESOLU][2];

   float minx = 0.0;
   float miny = 0.0;
   float maxx = size_.width();
   float maxy = size_.height();

   float minxi = minx + border;
   float maxxi = maxx - border;
   float minyi = miny + border;
   float maxyi = maxy - border;

   float facxi = (maxxi != minxi) ? 1.0f / (maxxi - minxi) : 0.0f;
   float facyi = (maxyi != minyi) ? 1.0f / (maxyi - minyi) : 0.0f;

   int corner = round_type();
   int minsize = 0;
   const int hnum = (
   (corner & (RoundTopLeft | RoundTopRight)) == (RoundTopLeft | RoundTopRight)
   ||
   (corner & (RoundBottomRight	| RoundBottomLeft))	== (RoundBottomRight | RoundBottomLeft)
   ) ? 1 : 2;
   const int vnum = (
   (corner & (RoundTopLeft | RoundBottomLeft)) == (RoundTopLeft | RoundBottomLeft)
   ||
   (corner & (RoundTopRight | RoundBottomRight)) == (RoundTopRight | RoundBottomRight)
   ) ? 1 : 2;

   float offset = 0.f;

   int count = 0;
   while (corner != 0) {
   count += corner & 0x1;
   corner = corner >> 1;
   }
   unsigned int outline_vertex_number = 4 - count + count * WIDGET_CURVE_RESOLU;
   corner = round_type();

   minsize = std::min(size_.width() * hnum, size_.height() * vnum);

   if (2.0f * rad > minsize)
   rad = 0.5f * minsize;

   if (2.0f * (radi + border) > minsize)
   radi = 0.5f * minsize - border * AbstractWindow::theme()->pixel();	// U.pixelsize;

   // mult
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++) {
   veci[i][0] = radi * cornervec[i][0];
   veci[i][1] = radi * cornervec[i][1];
   vec[i][0] = rad * cornervec[i][0];
   vec[i][1] = rad * cornervec[i][1];
   }

   {	// generate inner vertices

   if(inner_ptr->size() != ((outline_vertex_number + 2) * 3)) {
   inner_ptr->resize((outline_vertex_number + 2) * 3);
   }

   // inner_ptr[0, 0] is the center of a triangle fan
   (*inner_ptr)[0] = minxi + (maxxi - minxi) / 2.f;
   (*inner_ptr)[1] = minyi + (maxyi - minyi) / 2.f;

   if (shadedir == Vertical) {
   offset = make_shaded_offset(shadetop, shadedown,
   facyi * ((*inner_ptr)[1] - minyi));
   } else {
   offset = make_shaded_offset(shadetop, shadedown,
   facxi * ((*inner_ptr)[0] - minxi));
   }
   (*inner_ptr)[2] = offset;

   count = 1;

   // corner left-bottom
   if (corner & RoundBottomLeft) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   (*inner_ptr)[count * 3 + 0] = minxi + veci[i][1];
   (*inner_ptr)[count * 3 + 1] = minyi + radi - veci[i][0];

   if (shadedir == Vertical) {
   offset = make_shaded_offset(shadetop, shadedown,
   facyi * ((*inner_ptr)[count * 3 + 1] - minyi));
   } else {
   offset = make_shaded_offset(shadetop, shadedown,
   facxi * ((*inner_ptr)[count * 3 + 0] - minxi));
   }

   (*inner_ptr)[count * 3 + 2] = offset;
   }
   } else {
   (*inner_ptr)[count * 3 + 0] = minxi;
   (*inner_ptr)[count * 3 + 1] = minyi;

   if (shadedir == Vertical) {
   offset = make_shaded_offset(shadetop, shadedown, 0.f);
   } else {
   offset = make_shaded_offset(shadetop, shadedown, 0.f);
   }
   (*inner_ptr)[count * 3 + 2] = offset;

   count++;
   }

   // corner right-bottom
   if (corner & RoundBottomRight) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   (*inner_ptr)[count * 3 + 0] = maxxi - radi + veci[i][0];
   (*inner_ptr)[count * 3 + 1] = minyi + veci[i][1];

   if (shadedir == Vertical) {
   offset = make_shaded_offset(shadetop, shadedown,
   facyi * ((*inner_ptr)[count * 3 + 1] - minyi));
   } else {
   offset = make_shaded_offset(shadetop, shadedown,
   facxi * ((*inner_ptr)[count * 3 + 0] - minxi));
   }
   (*inner_ptr)[count * 3 + 2] = offset;
   }
   } else {
   (*inner_ptr)[count * 3 + 0] = maxxi;
   (*inner_ptr)[count * 3 + 1] = minyi;

   if (shadedir == Vertical) {
   offset = make_shaded_offset(shadetop, shadedown, 0.0f);
   } else {
   offset = make_shaded_offset(shadetop, shadedown, 1.0f);
   }
   (*inner_ptr)[count * 3 + 2] = offset;

   count++;
   }

   // corner right-top
   if (corner & RoundTopRight) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   (*inner_ptr)[count * 3 + 0] = maxxi - veci[i][1];
   (*inner_ptr)[count * 3 + 1] = maxyi - radi + veci[i][0];

   if (shadedir == Vertical) {
   offset = make_shaded_offset(shadetop, shadedown,
   facyi * ((*inner_ptr)[count * 3 + 1] - minyi));
   } else {
   offset = make_shaded_offset(shadetop, shadedown,
   facxi * ((*inner_ptr)[count * 3 + 0] - minxi));
   }
   (*inner_ptr)[count * 3 + 2] = offset;
   }
   } else {
   (*inner_ptr)[count * 3 + 0] = maxxi;
   (*inner_ptr)[count * 3 + 1] = maxyi;

   if (shadedir == Vertical) {
   offset = make_shaded_offset(shadetop, shadedown, 1.0f);
   } else {
   offset = make_shaded_offset(shadetop, shadedown, 1.0f);
   }
   (*inner_ptr)[count * 3 + 2] = offset;

   count++;
   }

   // corner left-top
   if (corner & RoundTopLeft) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   (*inner_ptr)[count * 3 + 0] = minxi + radi - veci[i][0];
   (*inner_ptr)[count * 3 + 1] = maxyi - veci[i][1];

   if (shadedir == Vertical) {
   offset = make_shaded_offset(shadetop, shadedown,
   facyi * ((*inner_ptr)[count * 3 + 1] - minyi));
   } else {
   offset = make_shaded_offset(shadetop, shadedown,
   facxi * ((*inner_ptr)[count * 3 + 0] - minxi));
   }
   (*inner_ptr)[count * 3 + 2] = offset;
   }
   } else {

   (*inner_ptr)[count * 3 + 0] = minxi;
   (*inner_ptr)[count * 3 + 1] = maxyi;

   if (shadedir == Vertical) {
   offset = make_shaded_offset(shadetop, shadedown, 1.0f);
   } else {
   offset = make_shaded_offset(shadetop, shadedown, 0.0f);
   }
   (*inner_ptr)[count * 3 + 2] = offset;

   count++;
   }

   (*inner_ptr)[count * 3 + 0] = (*inner_ptr)[3 + 0];
   (*inner_ptr)[count * 3 + 1] = (*inner_ptr)[3 + 1];
   (*inner_ptr)[count * 3 + 2] = (*inner_ptr)[3 + 2];

   }

   if(outer) {

   if (border > 0.f) {

   std::vector<GLfloat> edge_vertices(outline_vertex_number * 2);

   count = 0;

   // corner left-bottom
   if (corner & RoundBottomLeft) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   edge_vertices[count * 2 + 0] = minx + vec[i][1];
   edge_vertices[count * 2 + 1] = miny + rad - vec[i][0];
   }
   } else {
   edge_vertices[count * 2 + 0] = minx;
   edge_vertices[count * 2 + 1] = miny;
   count++;
   }

   // corner right-bottom
   if (corner & RoundBottomRight) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   edge_vertices[count * 2 + 0] = maxx - rad + vec[i][0];
   edge_vertices[count * 2 + 1] = miny + vec[i][1];
   }
   } else {
   edge_vertices[count * 2 + 0] = maxx;
   edge_vertices[count * 2 + 1] = miny;
   count++;
   }

   // m_half = count;

   // corner right-top
   if (corner & RoundTopRight) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   edge_vertices[count * 2 + 0] = maxx - vec[i][1];
   edge_vertices[count * 2 + 1] = maxy - rad + vec[i][0];
   }
   } else {
   edge_vertices[count * 2 + 0] = maxx;
   edge_vertices[count * 2 + 1] = maxy;
   count++;
   }

   // corner left-top
   if (corner & RoundTopLeft) {
   for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
   edge_vertices[count * 2 + 0] = minx + rad - vec[i][0];
   edge_vertices[count * 2 + 1] = maxy - vec[i][1];
   }
   } else {
   edge_vertices[count * 2 + 0] = minx;
   edge_vertices[count * 2 + 1] = maxy;
   count++;
   }

   GenerateTriangleStripVertices(inner_ptr, &edge_vertices, count, outer);

   } else {

   outer->clear();

   }

   }

   if(inner == 0) {
   delete inner_ptr;
   }

   }

   */

  void AbstractView::GenerateTriangleStripVertices (const std::vector<GLfloat>* inner,
                                                    const std::vector<GLfloat>* edge,
                                                    unsigned int num,
                                                    std::vector<GLfloat>* strip)
  {
    if (num > edge->size() / 2) {
      DBG_PRINT_MSG("Attempt to process %u vertices, but maximum is %ld", num,
                    edge->size() / 2);
      return;
    }

    if (strip->size() != (num * 2 + 2) * 2) {
      strip->resize((num * 2 + 2) * 2);
    }

    size_t count = 0;
    for (int i = 0, j = 0; count < num * 2; count++) {
      if (count % 2 == 0) {
        (*strip)[count * 2] = (*inner)[3 + i];
        (*strip)[count * 2 + 1] = (*inner)[3 + i + 1];
        i += 3;
      } else {
        (*strip)[count * 2] = (*edge)[j];
        (*strip)[count * 2 + 1] = (*edge)[j + 1];
        j += 2;
      }
    }

    (*strip)[count * 2] = (*inner)[3 + 0];
    (*strip)[count * 2 + 1] = (*inner)[3 + 1];
    (*strip)[count * 2 + 2] = (*edge)[0];
    (*strip)[count * 2 + 3] = (*edge)[1];
  }

  void AbstractView::GenerateVertices (const Size& size,
                                       float border,
                                       int round_type,
                                       float radius,
                                       std::vector<GLfloat>* inner,
                                       std::vector<GLfloat>* outer)
  {
    if (inner == nullptr && outer == nullptr) return;

    std::vector<GLfloat>* inner_ptr = nullptr;

    if (inner == nullptr) {
      inner_ptr = new std::vector<GLfloat>;
    } else {
      inner_ptr = inner;
    }

    border *= AbstractWindow::theme()->pixel();

    float rad = radius * AbstractWindow::theme()->pixel();
    float radi = rad - border;

    float vec[WIDGET_CURVE_RESOLU][2], veci[WIDGET_CURVE_RESOLU][2];

    float minx = 0.0;
    float miny = 0.0;
    float maxx = size.width();
    float maxy = size.height();

    float minxi = minx + border;		// U.pixelsize; // boundbox inner
    float maxxi = maxx - border; 	// U.pixelsize;
    float minyi = miny + border;		// U.pixelsize;
    float maxyi = maxy - border;		// U.pixelsize;

    int minsize = 0;
    int corner = round_type;
    const int hnum =
        ((corner & (RoundTopLeft | RoundTopRight))
            == (RoundTopLeft | RoundTopRight)
            || (corner & (RoundBottomRight | RoundBottomLeft))
                == (RoundBottomRight | RoundBottomLeft)) ? 1 : 2;
    const int vnum =
        ((corner & (RoundTopLeft | RoundBottomLeft))
            == (RoundTopLeft | RoundBottomLeft)
            || (corner & (RoundTopRight | RoundBottomRight))
                == (RoundTopRight | RoundBottomRight)) ? 1 : 2;

    int count = 0;
    while (corner != 0) {
      count += corner & 0x1;
      corner = corner >> 1;
    }
    unsigned int outline_vertex_number = 4 - count + count * WIDGET_CURVE_RESOLU;
    corner = round_type;

    minsize = std::min(size.width() * hnum, size.height() * vnum);

    if (2.0f * rad > minsize) rad = 0.5f * minsize;

    if (2.0f * (radi + border) > minsize) radi = 0.5f * minsize - border;	// U.pixelsize;

    // mult
    for (int i = 0; i < WIDGET_CURVE_RESOLU; i++) {
      veci[i][0] = radi * cornervec[i][0];
      veci[i][1] = radi * cornervec[i][1];
      vec[i][0] = rad * cornervec[i][0];
      vec[i][1] = rad * cornervec[i][1];
    }

    {	// generate inner vertices
      if (inner_ptr->size() != ((outline_vertex_number + 2) * 3)) {
        inner_ptr->resize((outline_vertex_number + 2) * 3);
      }

      // inner_ptr[0, 0] is the center of a triangle fan
      ((*inner_ptr))[0] = minxi + (maxxi - minxi) / 2.f;
      (*inner_ptr)[1] = minyi + (maxyi - minyi) / 2.f;
      (*inner_ptr)[2] = 0.f;

      count = 1;

      // corner left-bottom
      if (corner & RoundBottomLeft) {
        for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
          (*inner_ptr)[count * 3] = minxi + veci[i][1];
          (*inner_ptr)[count * 3 + 1] = minyi + radi - veci[i][0];
          (*inner_ptr)[count * 3 + 2] = 0.f;
        }
      } else {
        (*inner_ptr)[count * 3] = minxi;
        (*inner_ptr)[count * 3 + 1] = minyi;
        (*inner_ptr)[count * 3 + 2] = 0.f;
        count++;
      }

      // corner right-bottom
      if (corner & RoundBottomRight) {
        for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
          (*inner_ptr)[count * 3] = maxxi - radi + veci[i][0];
          (*inner_ptr)[count * 3 + 1] = minyi + veci[i][1];
          (*inner_ptr)[count * 3 + 2] = 0.f;
        }
      } else {
        (*inner_ptr)[count * 3] = maxxi;
        (*inner_ptr)[count * 3 + 1] = minyi;
        (*inner_ptr)[count * 3 + 2] = 0.f;
        count++;
      }

      // corner right-top
      if (corner & RoundTopRight) {
        for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
          (*inner_ptr)[count * 3] = maxxi - veci[i][1];
          (*inner_ptr)[count * 3 + 1] = maxyi - radi + veci[i][0];
          (*inner_ptr)[count * 3 + 2] = 0.f;
        }
      } else {
        (*inner_ptr)[count * 3] = maxxi;
        (*inner_ptr)[count * 3 + 1] = maxyi;
        (*inner_ptr)[count * 3 + 2] = 0.f;
        count++;
      }

      // corner left-top
      if (corner & RoundTopLeft) {
        for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
          (*inner_ptr)[count * 3] = minxi + radi - veci[i][0];
          (*inner_ptr)[count * 3 + 1] = maxyi - veci[i][1];
          (*inner_ptr)[count * 3 + 2] = 0.f;
        }

      } else {
        (*inner_ptr)[count * 3] = minxi;
        (*inner_ptr)[count * 3 + 1] = maxyi;
        (*inner_ptr)[count * 3 + 2] = 0.f;
        count++;
      }

      (*inner_ptr)[count * 3] = (*inner_ptr)[3 + 0];
      (*inner_ptr)[count * 3 + 1] = (*inner_ptr)[3 + 1];
      (*inner_ptr)[count * 3 + 2] = 0.f;
    }

    if (outer != nullptr) {

      if (border > 0.f) {

        std::vector<GLfloat> edge_vertices(outline_vertex_number * 2);

        count = 0;

        // corner left-bottom
        if (corner & RoundBottomLeft) {
          for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
            edge_vertices[count * 2] = minx + vec[i][1];
            edge_vertices[count * 2 + 1] = miny + rad - vec[i][0];
          }
        } else {
          edge_vertices[count * 2] = minx;
          edge_vertices[count * 2 + 1] = miny;
          count++;
        }

        // corner right-bottom
        if (corner & RoundBottomRight) {
          for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
            edge_vertices[count * 2] = maxx - rad + vec[i][0];
            edge_vertices[count * 2 + 1] = miny + vec[i][1];
          }
        } else {
          edge_vertices[count * 2] = maxx;
          edge_vertices[count * 2 + 1] = miny;
          count++;
        }

        // m_half = count;

        // corner right-top
        if (corner & RoundTopRight) {
          for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
            edge_vertices[count * 2] = maxx - vec[i][1];
            edge_vertices[count * 2 + 1] = maxy - rad + vec[i][0];
          }
        } else {
          edge_vertices[count * 2] = maxx;
          edge_vertices[count * 2 + 1] = maxy;
          count++;
        }

        // corner left-top
        if (corner & RoundTopLeft) {
          for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
            edge_vertices[count * 2] = minx + rad - vec[i][0];
            edge_vertices[count * 2 + 1] = maxy - vec[i][1];
          }
        } else {
          edge_vertices[count * 2] = minx;
          edge_vertices[count * 2 + 1] = maxy;
          count++;
        }

        GenerateTriangleStripVertices(inner_ptr, &edge_vertices, count, outer);

      } else {

        outer->clear();

      }

    }

    if (inner == nullptr) {
      delete inner_ptr;
    }
  }

  void AbstractView::GenerateVertices (const Size& size,
                                       float border,
                                       int round_type,
                                       float radius,
                                       Orientation shadedir,
                                       short shadetop,
                                       short shadedown,
                                       std::vector<GLfloat>* inner,
                                       std::vector<GLfloat>* outer)
  {
    if (inner == nullptr && outer == nullptr) return;

    std::vector<GLfloat>* inner_ptr = nullptr;

    if (inner == nullptr) {
      inner_ptr = new std::vector<GLfloat>;
    } else {
      inner_ptr = inner;
    }

    border *= AbstractWindow::theme()->pixel();

    float rad = radius * AbstractWindow::theme()->pixel();
    float radi = rad - border;

    float vec[WIDGET_CURVE_RESOLU][2], veci[WIDGET_CURVE_RESOLU][2];

    float minx = 0.0;
    float miny = 0.0;
    float maxx = size.width();
    float maxy = size.height();

    float minxi = minx + border;
    float maxxi = maxx - border;
    float minyi = miny + border;
    float maxyi = maxy - border;

    float facxi = (maxxi != minxi) ? 1.0f / (maxxi - minxi) : 0.0f;
    float facyi = (maxyi != minyi) ? 1.0f / (maxyi - minyi) : 0.0f;

    int corner = round_type;
    int minsize = 0;
    const int hnum =
        ((corner & (RoundTopLeft | RoundTopRight))
            == (RoundTopLeft | RoundTopRight)
            || (corner & (RoundBottomRight | RoundBottomLeft))
                == (RoundBottomRight | RoundBottomLeft)) ? 1 : 2;
    const int vnum =
        ((corner & (RoundTopLeft | RoundBottomLeft))
            == (RoundTopLeft | RoundBottomLeft)
            || (corner & (RoundTopRight | RoundBottomRight))
                == (RoundTopRight | RoundBottomRight)) ? 1 : 2;

    float offset = 0.f;

    int count = 0;
    while (corner != 0) {
      count += corner & 0x1;
      corner = corner >> 1;
    }
    unsigned int outline_vertex_number = 4 - count + count * WIDGET_CURVE_RESOLU;
    corner = round_type;

    minsize = std::min(size.width() * hnum, size.height() * vnum);

    if (2.0f * rad > minsize) rad = 0.5f * minsize;

    if (2.0f * (radi + border) > minsize)
      radi = 0.5f * minsize - border * AbstractWindow::theme()->pixel();// U.pixelsize;

          // mult
    for (int i = 0; i < WIDGET_CURVE_RESOLU; i++) {
      veci[i][0] = radi * cornervec[i][0];
      veci[i][1] = radi * cornervec[i][1];
      vec[i][0] = rad * cornervec[i][0];
      vec[i][1] = rad * cornervec[i][1];
    }

    {	// generate inner vertices

      if (inner_ptr->size() != ((outline_vertex_number + 2) * 3)) {
        inner_ptr->resize((outline_vertex_number + 2) * 3);
      }

      // inner_ptr[0, 0] is the center of a triangle fan
      (*inner_ptr)[0] = minxi + (maxxi - minxi) / 2.f;
      (*inner_ptr)[1] = minyi + (maxyi - minyi) / 2.f;

      if (shadedir == Vertical) {
        offset = make_shaded_offset(shadetop, shadedown,
                                    facyi * ((*inner_ptr)[1] - minyi));
      } else {
        offset = make_shaded_offset(shadetop, shadedown,
                                    facxi * ((*inner_ptr)[0] - minxi));
      }
      (*inner_ptr)[2] = offset;

      count = 1;

      // corner left-bottom
      if (corner & RoundBottomLeft) {
        for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
          (*inner_ptr)[count * 3 + 0] = minxi + veci[i][1];
          (*inner_ptr)[count * 3 + 1] = minyi + radi - veci[i][0];

          if (shadedir == Vertical) {
            offset = make_shaded_offset(
                shadetop, shadedown,
                facyi * ((*inner_ptr)[count * 3 + 1] - minyi));
          } else {
            offset = make_shaded_offset(
                shadetop, shadedown,
                facxi * ((*inner_ptr)[count * 3 + 0] - minxi));
          }

          (*inner_ptr)[count * 3 + 2] = offset;
        }
      } else {
        (*inner_ptr)[count * 3 + 0] = minxi;
        (*inner_ptr)[count * 3 + 1] = minyi;

        if (shadedir == Vertical) {
          offset = make_shaded_offset(shadetop, shadedown, 0.f);
        } else {
          offset = make_shaded_offset(shadetop, shadedown, 0.f);
        }
        (*inner_ptr)[count * 3 + 2] = offset;

        count++;
      }

      // corner right-bottom
      if (corner & RoundBottomRight) {
        for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
          (*inner_ptr)[count * 3 + 0] = maxxi - radi + veci[i][0];
          (*inner_ptr)[count * 3 + 1] = minyi + veci[i][1];

          if (shadedir == Vertical) {
            offset = make_shaded_offset(
                shadetop, shadedown,
                facyi * ((*inner_ptr)[count * 3 + 1] - minyi));
          } else {
            offset = make_shaded_offset(
                shadetop, shadedown,
                facxi * ((*inner_ptr)[count * 3 + 0] - minxi));
          }
          (*inner_ptr)[count * 3 + 2] = offset;
        }
      } else {
        (*inner_ptr)[count * 3 + 0] = maxxi;
        (*inner_ptr)[count * 3 + 1] = minyi;

        if (shadedir == Vertical) {
          offset = make_shaded_offset(shadetop, shadedown, 0.0f);
        } else {
          offset = make_shaded_offset(shadetop, shadedown, 1.0f);
        }
        (*inner_ptr)[count * 3 + 2] = offset;

        count++;
      }

      // corner right-top
      if (corner & RoundTopRight) {
        for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
          (*inner_ptr)[count * 3 + 0] = maxxi - veci[i][1];
          (*inner_ptr)[count * 3 + 1] = maxyi - radi + veci[i][0];

          if (shadedir == Vertical) {
            offset = make_shaded_offset(
                shadetop, shadedown,
                facyi * ((*inner_ptr)[count * 3 + 1] - minyi));
          } else {
            offset = make_shaded_offset(
                shadetop, shadedown,
                facxi * ((*inner_ptr)[count * 3 + 0] - minxi));
          }
          (*inner_ptr)[count * 3 + 2] = offset;
        }
      } else {
        (*inner_ptr)[count * 3 + 0] = maxxi;
        (*inner_ptr)[count * 3 + 1] = maxyi;

        if (shadedir == Vertical) {
          offset = make_shaded_offset(shadetop, shadedown, 1.0f);
        } else {
          offset = make_shaded_offset(shadetop, shadedown, 1.0f);
        }
        (*inner_ptr)[count * 3 + 2] = offset;

        count++;
      }

      // corner left-top
      if (corner & RoundTopLeft) {
        for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
          (*inner_ptr)[count * 3 + 0] = minxi + radi - veci[i][0];
          (*inner_ptr)[count * 3 + 1] = maxyi - veci[i][1];

          if (shadedir == Vertical) {
            offset = make_shaded_offset(
                shadetop, shadedown,
                facyi * ((*inner_ptr)[count * 3 + 1] - minyi));
          } else {
            offset = make_shaded_offset(
                shadetop, shadedown,
                facxi * ((*inner_ptr)[count * 3 + 0] - minxi));
          }
          (*inner_ptr)[count * 3 + 2] = offset;
        }
      } else {

        (*inner_ptr)[count * 3 + 0] = minxi;
        (*inner_ptr)[count * 3 + 1] = maxyi;

        if (shadedir == Vertical) {
          offset = make_shaded_offset(shadetop, shadedown, 1.0f);
        } else {
          offset = make_shaded_offset(shadetop, shadedown, 0.0f);
        }
        (*inner_ptr)[count * 3 + 2] = offset;

        count++;
      }

      (*inner_ptr)[count * 3 + 0] = (*inner_ptr)[3 + 0];
      (*inner_ptr)[count * 3 + 1] = (*inner_ptr)[3 + 1];
      (*inner_ptr)[count * 3 + 2] = (*inner_ptr)[3 + 2];

    }

    if (outer != nullptr) {

      if (border > 0.f) {

        std::vector<GLfloat> edge_vertices(outline_vertex_number * 2);

        count = 0;

        // corner left-bottom
        if (corner & RoundBottomLeft) {
          for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
            edge_vertices[count * 2 + 0] = minx + vec[i][1];
            edge_vertices[count * 2 + 1] = miny + rad - vec[i][0];
          }
        } else {
          edge_vertices[count * 2 + 0] = minx;
          edge_vertices[count * 2 + 1] = miny;
          count++;
        }

        // corner right-bottom
        if (corner & RoundBottomRight) {
          for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
            edge_vertices[count * 2 + 0] = maxx - rad + vec[i][0];
            edge_vertices[count * 2 + 1] = miny + vec[i][1];
          }
        } else {
          edge_vertices[count * 2 + 0] = maxx;
          edge_vertices[count * 2 + 1] = miny;
          count++;
        }

        // m_half = count;

        // corner right-top
        if (corner & RoundTopRight) {
          for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
            edge_vertices[count * 2 + 0] = maxx - vec[i][1];
            edge_vertices[count * 2 + 1] = maxy - rad + vec[i][0];
          }
        } else {
          edge_vertices[count * 2 + 0] = maxx;
          edge_vertices[count * 2 + 1] = maxy;
          count++;
        }

        // corner left-top
        if (corner & RoundTopLeft) {
          for (int i = 0; i < WIDGET_CURVE_RESOLU; i++, count++) {
            edge_vertices[count * 2 + 0] = minx + rad - vec[i][0];
            edge_vertices[count * 2 + 1] = maxy - vec[i][1];
          }
        } else {
          edge_vertices[count * 2 + 0] = minx;
          edge_vertices[count * 2 + 1] = maxy;
          count++;
        }

        GenerateTriangleStripVertices(inner_ptr, &edge_vertices, count, outer);

      } else {

        outer->clear();

      }

    }

    if (inner == nullptr) {
      delete inner_ptr;
    }

  }

  AbstractView* AbstractView::operator [] (int i) const
  {
    if ((i < 0) || (i >= subs_count_)) return 0;

    AbstractView* widget = 0;

    if (i < ((subs_count_ + 1) / 2)) {

      widget = first_subview_;
      while (i > 0) {
        widget = widget->next_view_;
        i--;
      }

    } else {

      widget = last_subview_;
      int max = subs_count_ - 1;
      while (i < max) {
        widget = widget->previous_view_;
        i++;
      }

    }

    return widget;
  }

  AbstractView* AbstractView::GetSubViewAt (int i) const
  {
    if ((i < 0) || (i >= subs_count_)) return 0;

    AbstractView* widget = 0;

    if (i < ((subs_count_ + 1) / 2)) {

      widget = first_subview_;
      while (i > 0) {
        widget = widget->next_view_;
        i--;
      }

    } else {

      widget = last_subview_;
      int max = subs_count_ - 1;
      while (i < max) {
        widget = widget->previous_view_;
        i++;
      }

    }

    //assert(widget != 0);

    return widget;
  }

  bool AbstractView::PushFrontSubView (AbstractView* view)
  {
    if (!view) return false;

    if (view->superview_) {

      if (view->superview_ == this) {
        DBG_PRINT_MSG("AbstractRoundWidget %s is already in container %s",
                      view->name_.c_str(), view->superview_->name().c_str());
        return true;
      } else {
        // Set widget's container to 0
        view->superview_->RemoveSubView(view);
      }

    }

    assert(view->previous_view_ == 0);
    assert(view->next_view_ == 0);
    assert(view->superview_ == 0);

    if (first_subview_) {
      first_subview_->previous_view_ = view;
      view->next_view_ = first_subview_;
    } else {
      assert(last_subview_ == 0);
      view->next_view_ = 0;
      last_subview_ = view;
    }
    first_subview_ = view;

    view->previous_view_ = 0;
    view->superview_ = this;
    subs_count_++;

    view->PerformAfterAdded();

    return true;
  }

  bool AbstractView::InsertSubView (int index, AbstractView* view)
  {
    if (!view) return false;

    if (view->superview_) {

      if (view->superview_ == this) {
        DBG_PRINT_MSG("AbstractRoundWidget %s is already in container %s",
                      view->name_.c_str(), view->superview_->name().c_str());
        return true;
      } else {
        // Set widget's container to 0
        view->superview_->RemoveSubView(view);
      }

    }

    assert(view->previous_view_ == 0);
    assert(view->next_view_ == 0);
    assert(view->superview_ == 0);

    if (first_subview_ == 0) {
      assert(last_subview_ == 0);

      view->next_view_ = 0;
      last_subview_ = view;
      first_subview_ = view;
      view->previous_view_ = 0;

    } else {

      AbstractView* p = first_subview_;

      if (index > 0) {

        while (p && (index > 0)) {
          if (p->next_view_ == 0) break;

          p = p->next_view_;
          index--;
        }

        if (index == 0) {	// insert

          view->previous_view_ = p->previous_view_;
          view->next_view_ = p;
          p->previous_view_->next_view_ = view;
          p->previous_view_ = view;

        } else {	// same as push back

          assert(p == last_subview_);
          last_subview_->next_view_ = view;
          view->previous_view_ = last_subview_;
          last_subview_ = view;
          view->next_view_ = 0;

        }

      } else {	// same as push front

        first_subview_->previous_view_ = view;
        view->next_view_ = first_subview_;
        first_subview_ = view;
        view->previous_view_ = 0;

      }

    }

    view->superview_ = this;
    subs_count_++;

    view->PerformAfterAdded();

    return true;
  }

  bool AbstractView::PushBackSubView (AbstractView* view)
  {
    if (!view) return false;

    if (view->superview_) {

      if (view->superview_ == this) {
        DBG_PRINT_MSG("AbstractRoundWidget %s is already in container %s",
                      view->name_.c_str(), view->superview_->name().c_str());
        return true;
      } else {
        // Set widget's container to 0
        view->superview_->RemoveSubView(view);
      }

    }

    assert(view->previous_view_ == 0);
    assert(view->next_view_ == 0);
    assert(view->superview_ == 0);

    if (last_subview_) {
      last_subview_->next_view_ = view;
      view->previous_view_ = last_subview_;
    } else {
      assert(first_subview_ == 0);
      view->previous_view_ = 0;
      first_subview_ = view;
    }
    last_subview_ = view;

    view->next_view_ = 0;
    view->superview_ = this;
    subs_count_++;

    view->PerformAfterAdded();
    assert(view->superview_ == this);

    return true;
  }

  bool AbstractView::RemoveSubView (AbstractView* view)
  {
    if (!view) return false;

    assert(view->superview_ == this);
    view->PerformBeforeRemoved();
    assert(view->superview_ == this);

    if (view->previous_view_) {
      view->previous_view_->next_view_ = view->next_view_;
    } else {
      assert(first_subview_ == view);
      first_subview_ = view->next_view_;
    }

    if (view->next_view_) {
      view->next_view_->previous_view_ = view->previous_view_;
    } else {
      assert(last_subview_ == view);
      last_subview_ = view->previous_view_;
    }

    subs_count_--;
    assert(subs_count_ >= 0);

    view->previous_view_ = 0;
    view->next_view_ = 0;
    view->superview_ = 0;

    return true;
  }

  void AbstractView::PerformAfterAdded ()
  {

  }

  void AbstractView::PerformBeforeRemoved ()
  {

  }

  void AbstractView::ClearSubViews ()
  {
    AbstractView* ptr = first_subview_;
    AbstractView* next_ptr = 0;

    while (ptr) {

      next_ptr = ptr->next_view_;

      ptr->previous_view_ = 0;
      ptr->next_view_ = 0;
      ptr->superview_ = 0;

      if (ptr->managed()) {

        if (ptr->reference_count() == 0) {
          delete ptr;
        } else {
          DBG_PRINT_MSG(
              "%s is set managed but is referenced by another object, it will be deleted later",
              ptr->name_.c_str());
        }

      } else {

        if (ptr->reference_count() == 0) {
          DBG_PRINT_MSG(
              "Warning: %s is not set managed and will not be deleted",
              ptr->name_.c_str());
        } else {
          DBG_PRINT_MSG(
              "%s is set unmanaged but is referenced by another object, it will be deleted later",
              ptr->name_.c_str());
        }

      }

      ptr = next_ptr;
    }

    subs_count_ = 0;
    first_subview_ = 0;
    last_subview_ = 0;
  }

  void AbstractView::ResizeSubView (AbstractView* sub, int width, int height)
  {
    if (!sub || sub->superview() != this) return;

    if (sub->size().width() == width && sub->size().height() == height) return;

    Size new_size(width, height);
    SizeUpdateRequest request(this, sub, &new_size);

    if (sub->SizeUpdateTest(request)) {
      sub->PerformSizeUpdate(request);
      sub->set_size(width, height);
    }
  }

  void AbstractView::ResizeSubView (AbstractView* sub, const Size& size)
  {
    if (!sub || sub->superview() != this) return;

    if (sub->size() == size) return;

    SizeUpdateRequest request(this, sub, &size);

    if (sub->SizeUpdateTest(request)) {
      sub->PerformSizeUpdate(request);
      sub->set_size(size);
    }
  }

  void AbstractView::MoveSubViewTo (AbstractView* sub, int x, int y)
  {
    if (!sub || sub->superview() != this) return;

    if (sub->position().x() == x && sub->position().y() == y) return;

    Point new_pos(x, y);

    PositionUpdateRequest request(this, sub, &new_pos);

    if (sub->PositionUpdateTest(request)) {
      sub->PerformPositionUpdate(request);
      sub->set_position(x, y);
    }
  }

  void AbstractView::MoveSubViewTo (AbstractView* sub, const Point& pos)
  {
    if (!sub || sub->superview() != this) return;

    if (sub->position() == pos) return;

    PositionUpdateRequest request(this, sub, &pos);

    if (sub->PositionUpdateTest(request)) {
      sub->PerformPositionUpdate(request);
      sub->set_position(pos);
    }
  }

  void AbstractView::SetSubViewVisibility (AbstractView* sub, bool visible)
  {
    if (!sub || sub->superview() != this) return;

    if (sub->visiable() == visible) return;

    VisibilityUpdateRequest request(this, sub, &visible);

    if (sub->VisibilityUpdateTest(request)) {
      sub->PerformVisibilityUpdate(request);
      sub->set_visible(visible);
    }
  }


  Response AbstractView::RecursiveDispatchKeyEvent (AbstractView* subview,
                                                     AbstractWindow* context)
  {
    if (subview == this) {
      return Ignore;
    } else {

      Response response = Ignore;

      if (subview->superview()) {
        response = RecursiveDispatchKeyEvent(subview->superview(), context);
        if (response == Finish) {
          return response;
        } else {
          return subview->PerformKeyPress(context);
        }
      } else {
        return subview->PerformKeyPress(context);
      }

    }
  }

  AbstractView* AbstractView::RecursiveDispatchMousePress (AbstractView* subview,
                                                            AbstractWindow* context)
  {
    if (subview == this) {
      return 0;
    } else {

      Response response = Ignore;
      AbstractView* ret_val = 0;

      if (subview->superview()) {

        ret_val = RecursiveDispatchMousePress(subview->superview(), context);

        if (ret_val == 0) {

          response = subview->PerformMousePress(context);

          return response == Finish ? subview : 0;

        } else {
          return ret_val;
        }

      } else {
        response = subview->PerformMousePress(context);
        return response == Finish ? subview : 0;
      }

    }
  }

  Response AbstractView::RecursiveDispatchMouseMoveEvent (AbstractView* subview,
                                                           AbstractWindow* context)
  {
    if (subview == this) {
      return Ignore;
    } else {

      if (subview->superview()) {
        if (RecursiveDispatchMouseMoveEvent(subview->superview(), context)
            == Ignore) {
          return subview->PerformMouseMove(context);
        } else {
          return Finish;
        }

      } else {
        return subview->PerformMouseMove(context);
      }

    }
  }

  Response AbstractView::RecursiveDispatchMouseReleaseEvent (AbstractView* subview,
                                                              AbstractWindow* context)
  {
    if (subview == this) {
      return Ignore;
    } else {

      if (subview->superview()) {
        if (RecursiveDispatchMouseReleaseEvent(subview->superview(), context)
            == Ignore) {
          return subview->PerformMouseRelease(context);
        } else {
          return Finish;
        }

      } else {
        DBG_PRINT_MSG("mouse press in %s", subview->name().c_str());
        return subview->PerformMouseRelease(context);
      }

    }
  }

  float AbstractView::make_shaded_offset (short shadetop,
                                          short shadedown,
                                          float fact)
  {
    float faci = glm::clamp(fact - 0.5f / 255.f, 0.f, 1.f);
    float facm = 1.f - fact;

    return faci * (shadetop / 255.f) + facm * (shadedown / 255.f);
  }

} /* namespace BlendInt */
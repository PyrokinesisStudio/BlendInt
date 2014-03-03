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

#include <iostream>
#include <stdlib.h>

#include <BlendInt/Types.hpp>

#include <BlendInt/Core/Size.hpp>

#include <BlendInt/OpenGL/GLTexture2D.hpp>
#include <BlendInt/OpenGL/GLFramebuffer.hpp>
#include <BlendInt/OpenGL/GLRenderbuffer.hpp>

#include <BlendInt/Gui/AbstractWidget.hpp>
#include <BlendInt/Service/ContextManager.hpp>

#include "../Intern/ScreenBuffer.hpp"

using std::cout;
using std::cerr;
using std::endl;

namespace BlendInt {

	ContextLayer::ContextLayer()
	: refresh(true), widgets(0), buffer(0)
	{

	}

	ContextLayer::~ContextLayer()
	{
		if(buffer) {
			std::cout << "Delete texture buffer in context layer" << std::endl;
			delete buffer;
		}

		if(widgets) {
			std::cout << "Delete widget set in context layer" << std::endl;

			/*
			set<AbstractWidget*>::iterator it;

			for(it = widgets->begin(); it != widgets->end(); it++)
			{
				if((*it)->count() == 0) delete *it;
			}
			*/

			widgets->clear();
		}
	}

	ContextManager* ContextManager::context_manager = 0;

	bool ContextManager::refresh_once = false;

	bool ContextManager::force_refresh_all = true;

	ContextManager* ContextManager::Instance ()
	{
#ifdef DEBUG
		if (!context_manager) {
			DBG_PRINT_MSG("%s", "The Context Manager is not initialized successfully! Exit");
			exit(EXIT_FAILURE);
		}
#endif

		return context_manager;
	}

	bool ContextManager::Initialize ()
	{
		bool result = true;

		if (!context_manager) {
			context_manager = new ContextManager;
		}

		if (!context_manager) {
			result = false;
		}

		return result;
	}

	void ContextManager::Release ()
	{
		if(context_manager) {
			delete context_manager;
			context_manager = 0;
		}
	}

	ContextManager::ContextManager ()
	: AbstractWidget(), m_main_buffer(0), m_screenbuffer(0)
	{
		m_screenbuffer = new ScreenBuffer;
		m_hover_deque.reset(new std::deque<AbstractWidget*>);

		m_main_buffer = new GLTexture2D;
	}

	ContextManager::~ContextManager ()
	{
		// set focus widget to 0
		if(AbstractWidget::focused_widget) {
			AbstractWidget::focused_widget->m_flag.reset(AbstractWidget::WidgetFlagFocus);
			AbstractWidget::focused_widget = 0;
		}

		map<int, ContextLayer>::iterator layer_iter;
		set<AbstractWidget*>::iterator widget_iter;
		set<AbstractWidget*>* widget_set_p = 0;

		for(layer_iter = m_layers.begin(); layer_iter != m_layers.end(); layer_iter++)
		{
			widget_set_p = layer_iter->second.widgets;
			for(widget_iter = widget_set_p->begin(); widget_iter != widget_set_p->end(); widget_iter++)
			{
				(*widget_iter)->destroyed().disconnectOne(this, &ContextManager::OnDestroyObject);
				//if((*widget_iter)->count() == 0) delete *widget_iter;
			}

			widget_set_p->clear();
		}

		m_deque.clear();
		if(m_screenbuffer) delete m_screenbuffer;

		m_layers.clear();
		m_index.clear();

		if(m_main_buffer) {
			m_main_buffer->Clear();
			delete m_main_buffer;
		}
	}

	bool ContextManager::Register (AbstractWidget* obj)
	{
		if (!obj) return false;

		if(obj->m_flag[AbstractWidget::WidgetFlagRegistered]) return true;

		AddWidget(obj);

		obj->m_flag.set(AbstractWidget::WidgetFlagRegistered);

		m_events->connect(obj->destroyed(), this, &ContextManager::OnDestroyObject);

		return true;
	}

	bool ContextManager::Unregister (AbstractWidget* obj)
	{
		if (!obj) return false;

		if(!obj->m_flag[AbstractWidget::WidgetFlagRegistered]) return true;

		if(!RemoveWidget(obj)) {
			obj->m_flag.reset(AbstractWidget::WidgetFlagRegistered);
			DBG_PRINT_MSG("object: %s is not in stored in layer %d in context manager", obj->name().c_str(), obj->z());
			return false;
		}

		obj->m_flag.reset(AbstractWidget::WidgetFlagRegistered);

		obj->destroyed().disconnectOne(this, &ContextManager::OnDestroyObject);

		return true;
	}

	void ContextManager::AddWidget (AbstractWidget* obj)
	{
//		map<AbstractWidget*, int>::iterator map_it;
//
//		map_it = m_index.find(obj);
//
//		if(map_it != m_index.end()) {
//			if (map_it->second == obj->z()) {
//				return false;
//			}
//
//			set<AbstractWidget*>* p = m_layers[map_it->second];
//			set<AbstractWidget*>::iterator it = p->find(obj);
//			if (it != p->end()) {
//				p->erase (it);
//			} else {
//#ifdef DEBUG
//				std::cerr << "Note: object is not recorded" << std::endl;
//#endif
//			}
//
//			if (p->empty()) {
//				m_layers.erase(map_it->second);
//				delete p;
//			}
//
//			map<int, set<AbstractWidget*>* >::iterator layer_it;
//			layer_it = m_layers.find(obj->z());
//			if(layer_it != m_layers.end()) {
//				layer_it->second->insert(obj);
//			} else {
//				set<AbstractWidget*>* new_set = new set<AbstractWidget*>;
//				new_set->insert(obj);
//				m_layers[obj->z()] = new_set;
//			}
//
//		} else {

			map<int, ContextLayer>::iterator layer_iter;
			layer_iter = m_layers.find(obj->z());
			if(layer_iter != m_layers.end()) {
				layer_iter->second.widgets->insert(obj);
			} else {
				set<AbstractWidget*>* new_widget_set_p = new set<AbstractWidget*>;
				new_widget_set_p->insert(obj);
				m_layers[obj->z()].widgets = new_widget_set_p;

				// Refresh this layer in the render loop
				m_layers[obj->z()].refresh = true;
				refresh_once = true;

			}
			
//		}

		m_index[obj] = obj->z();
		//return true;
	}

	bool ContextManager::RemoveWidget (AbstractWidget* obj)
	{
		if (!obj) return false;

		if(AbstractWidget::focused_widget == obj) {
			obj->m_flag.reset(AbstractWidget::WidgetFlagFocus);
			AbstractWidget::focused_widget = 0;
		}

		map<AbstractWidget*, int>::iterator index_iter;
		
		index_iter = m_index.find(obj);

		if(index_iter != m_index.end()) {

			int z = index_iter->second;

			set<AbstractWidget*>* widget_set_p = m_layers[z].widgets;
			set<AbstractWidget*>::iterator widget_iter = widget_set_p->find(obj);
			if (widget_iter != widget_set_p->end()) {
				widget_set_p->erase (widget_iter);
			} else {
				DBG_PRINT_MSG("Error: object %s is not recorded in set", obj->name().c_str());
			}

			if (widget_set_p->empty()) {
				//delete widget_set_p; widget_set_p = 0;
				delete m_layers[z].widgets; m_layers[z].widgets = 0;

				m_layers[z].buffer->Clear();
				delete m_layers[z].buffer; m_layers[z].buffer = 0;

				m_layers.erase(z);
			}

			m_index.erase(obj);

		} else {
			DBG_PRINT_MSG("Error: object %s is not recorded in map", obj->name().c_str());
			return false;
		}

		return true;
	}

	bool ContextManager::Update (int type, const void* data)
	{
		return true;
	}

	void ContextManager::CursorEnterEvent (bool entered)
	{
	}

	void ContextManager::KeyPressEvent (KeyEvent* event)
	{
	}

	void ContextManager::ContextMenuPressEvent (ContextMenuEvent* event)
	{
	}

	void ContextManager::ContextMenuReleaseEvent (ContextMenuEvent* event)
	{
	}

	void ContextManager::MousePressEvent (MouseEvent* event)
	{
	}

	void ContextManager::MouseReleaseEvent (MouseEvent* event)
	{
	}

	void ContextManager::MouseMoveEvent (MouseEvent* event)
	{
	}

	void ContextManager::Draw ()
	{
		m_deque.clear();

		if(force_refresh_all) {

			map<int, ContextLayer>::iterator layer_iter;
			unsigned int count = 0;
			set<AbstractWidget*>* widget_set_p = 0;

			for(layer_iter = m_layers.begin();
					layer_iter != m_layers.end();
					layer_iter++)
			{
				widget_set_p = layer_iter->second.widgets;

				DBG_PRINT_MSG("layer need to be refreshed: %d", layer_iter->first);

				if(!layer_iter->second.buffer) {
					layer_iter->second.buffer = new GLTexture2D;
					layer_iter->second.buffer->Generate();
				}

				OffScreenRenderToTexture (layer_iter->first, widget_set_p, layer_iter->second.buffer);
				m_deque.push_back(layer_iter->second.buffer);

				count++;

				layer_iter->second.refresh = false;
			}

			RenderToScreenBuffer();

			refresh_once = false;
			force_refresh_all = false;

		} else if(refresh_once) {

			map<int, ContextLayer>::iterator layer_iter;
			unsigned int count = 0;
			set<AbstractWidget*>* widget_set_p = 0;

			for(layer_iter = m_layers.begin();
					layer_iter != m_layers.end();
					layer_iter++)
			{
				widget_set_p = layer_iter->second.widgets;

				if(layer_iter->second.refresh) {

					DBG_PRINT_MSG("layer need to be refreshed: %d", layer_iter->first);

					if(!layer_iter->second.buffer) {
						layer_iter->second.buffer = new GLTexture2D;
						layer_iter->second.buffer->Generate();
					}
					OffScreenRenderToTexture (layer_iter->first, widget_set_p, layer_iter->second.buffer);

				} else {

					if(!layer_iter->second.buffer) {
						layer_iter->second.buffer = new GLTexture2D;
						layer_iter->second.buffer->Generate();

						OffScreenRenderToTexture (layer_iter->first, widget_set_p, layer_iter->second.buffer);
					} else if (layer_iter->second.buffer->id() == 0) {
						layer_iter->second.buffer->Generate();

						OffScreenRenderToTexture (layer_iter->first, widget_set_p, layer_iter->second.buffer);
					}
				}

				count++;

				m_deque.push_back(layer_iter->second.buffer);
				layer_iter->second.refresh = false;
			}

			RenderToScreenBuffer();

			refresh_once = false;
		}

		// ---------
		/*

		glClearColor(0.447, 0.447, 0.447, 1.00);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Here cannot enable depth test -- glEnable(GL_DEPTH_TEST);
		//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

//		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//		glEnable(GL_BLEND);

//		glViewport(0, 0, size().width(), size().height());
//		glMatrixMode(GL_PROJECTION);
//		glLoadIdentity();
//		glOrtho(0.f, (float) size().width(), 0.f, (float) size().width(), 100.f, -100.f);

//		glMatrixMode(GL_MODELVIEW);
//		glLoadIdentity();

		draw_grid(size().width(), size().height());

		map<int, ContextLayer>::iterator layer_iter;
		for(layer_iter = ContextManager::context_manager->m_layers.begin();
				layer_iter != ContextManager::context_manager->m_layers.end();
				layer_iter++)
		{
			m_screenbuffer->Render(layer_iter->second.buffer);
		}
		*/

		// ---------

		m_screenbuffer->Render(context_manager->m_main_buffer);
	}

#ifdef DEBUG

	void ContextManager::DrawGrid(int width, int height)
	{
		// Draw grid for debug
		const int small_step = 20;
		const int big_step = 100;

		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);

		glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
		glLineStipple(1, 0xAAAA);

		for (int num = 1; num < width; num++) {
			int step = num * small_step;
			glBegin(GL_LINES);
			glVertex2i(0, step);
			glVertex2i(width, step);
			glEnd();

		}

		for (int num = 1; num < height; num++) {
			int step = num * small_step;
			glBegin(GL_LINES);
			glVertex2i(step, 0);
			glVertex2i(step, height);
			glEnd();
		}

		glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
		glLineStipple(1, 0xAAAA);

		for (int num = 1; num < width; num++) {
			int step = num * big_step;
			glBegin(GL_LINES);
			glVertex2i(0, step);
			glVertex2i(width, step);
			glEnd();
		}

		for (int num = 1; num < height; num++) {
			int step = num * big_step;
			glBegin(GL_LINES);
			glVertex2i(step, 0);
			glVertex2i(step, height);
			glEnd();
		}

		glDisable(GL_LINE_STIPPLE);
	}

#endif

	void ContextManager::OffScreenRenderToTexture (int layer, std::set<AbstractWidget*>* widgets, GLTexture2D* texture)
	{
		GLsizei width = size().width();
		GLsizei height = size().height();

		// Create and set texture to render to.
		GLTexture2D* tex = texture;
		tex->Bind();
		tex->SetWrapMode(GL_REPEAT, GL_REPEAT);
		tex->SetMinFilter(GL_NEAREST);
		tex->SetMagFilter(GL_NEAREST);
		tex->SetImage(width, height, 0);

		// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
		GLFramebuffer* fb = new GLFramebuffer;
		fb->Generate();
		fb->Bind();

		// Set "renderedTexture" as our colour attachement #0
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		        GL_TEXTURE_2D, tex->id(), 0);
		//fb->Attach(*tex, GL_COLOR_ATTACHMENT0);

		GLuint rb = 0;
		glGenRenderbuffers(1, &rb);

		glBindRenderbuffer(GL_RENDERBUFFER, rb);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
		        width, height);

		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		        GL_RENDERBUFFER, rb);

		if(GLFramebuffer::CheckStatus()) {

			fb->Bind();

			glClearColor(0.0, 0.0, 0.0, 0.00);

			glClearDepth(1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			// Here cannot enable depth test -- glEnable(GL_DEPTH_TEST);
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

			glEnable(GL_BLEND);

			glViewport(0, 0, width, height);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0.f, (float) width, 0.f, (float) height, 100.f, -100.f);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			set<AbstractWidget*>::iterator widget_iter;

			for (widget_iter = widgets->begin(); widget_iter != widgets->end(); widget_iter++)
			{
				//(*set_it)->Draw();
				DispatchDrawEvent(*widget_iter);
			}

			// uncomment the code below to test the layer buffer (texture)
			/*
			std::string str("layer");
			char buf[4];
			sprintf(buf, "%d", layer);
			str = str + buf + ".png";
			tex->WriteToFile(str);
			 */
		}

		fb->Reset();
		tex->Reset();

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glDeleteRenderbuffers(1, &rb);

		fb->Reset();
		delete fb; fb = 0;

	}

	void ContextManager::RenderToScreenBuffer ()
	{
		GLsizei width = size().width();
		GLsizei height = size().height();

		// Create and set texture to render to.
		GLTexture2D* tex = ContextManager::m_main_buffer;
		tex->Generate();
		tex->Bind();
		tex->SetWrapMode(GL_REPEAT, GL_REPEAT);
		tex->SetMinFilter(GL_NEAREST);
		tex->SetMagFilter(GL_NEAREST);
		tex->SetImage(width, height, 0);

		// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
		GLFramebuffer* fb = new GLFramebuffer;
		fb->Generate();
		fb->Bind();

		// Set "renderedTexture" as our colour attachement #0
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, tex->id(), 0);
		//fb->Attach(*tex, GL_COLOR_ATTACHMENT0);

		GLuint rb = 0;
		glGenRenderbuffers(1, &rb);

		glBindRenderbuffer(GL_RENDERBUFFER, rb);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width,
		        height);

		//Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, rb);

		if (GLFramebuffer::CheckStatus()) {

			fb->Bind();

			/* -- old

			 PreDrawContext(true);

			 glViewport(0, 0, width, height);
			 glMatrixMode(GL_PROJECTION);
			 glLoadIdentity();
			 glOrtho(0.f, (float) width, 0.f, (float) height, 100.f, -100.f);

			 glMatrixMode(GL_MODELVIEW);
			 glLoadIdentity();

			 #ifdef DEBUG
			 //DrawTriangle(false);
			 draw_grid(width, height);
			 #endif

			 map<int, ContextLayer>::iterator layer_iter;
			 set<AbstractWidget*>::iterator widget_iter;

			 for(layer_iter = ContextManager::context_manager->m_layers.begin();
			 layer_iter != ContextManager::context_manager->m_layers.end();
			 layer_iter++)
			 {
			 set<AbstractWidget*>* pset = layer_iter->second.widgets;
			 for (widget_iter = pset->begin(); widget_iter != pset->end(); widget_iter++)
			 {
			 //(*set_it)->Draw();
			 DispatchDrawEvent(*widget_iter);
			 }
			 }

			 */

			PreDrawContext(true);

			glViewport(0, 0, width, height);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glOrtho(0.f, (float) width, 0.f, (float) height, 100.f, -100.f);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

#ifdef DEBUG
			//DrawTriangle(false);
			DrawGrid(width, height);
#endif

			/*
			 map<int, ContextLayer>::iterator layer_iter;
			 for(layer_iter = ContextManager::context_manager->m_layers.begin();
			 layer_iter != ContextManager::context_manager->m_layers.end();
			 layer_iter++)
			 {
			 m_screenbuffer->Render(layer_iter->second.buffer);
			 }
			 */
			std::deque<GLTexture2D*>::iterator it;
			for (it = m_deque.begin(); it != m_deque.end(); it++) {
				m_screenbuffer->Render(*it);
			}

		}

		fb->Reset();
		tex->Reset();

		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glDeleteRenderbuffers(1, &rb);

		fb->Reset();
		delete fb;
		fb = 0;

	}

	void ContextManager::PreDrawContext (bool fbo)
	{
		glClearColor(0.447, 0.447, 0.447, 1.00);

		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Here cannot enable depth test -- glEnable(GL_DEPTH_TEST);

		if(fbo) {
			glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		} else {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}

		glEnable(GL_BLEND);
	}

	void ContextManager::DispatchDrawEvent(AbstractWidget* widget)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		glTranslatef(widget->position().x(),
					 widget->position().y(),
					 widget->z());

		widget->Draw();

		glPopMatrix();

		for(std::set<AbstractWidget*>::iterator it = widget->m_branches.begin(); it != widget->m_branches.end(); it++)
		{
			DispatchDrawEvent(*it);
		}
	}

	void ContextManager::OnDestroyObject(AbstractWidget* obj)
	{
		std::cout << "Get event" << std::endl;
		if(!obj) return;

		RemoveWidget(obj);
		obj->m_flag.reset(AbstractWidget::WidgetFlagRegistered);
		obj->destroyed().disconnectOne(this, &ContextManager::OnDestroyObject);

		// TODO: remove this widget and its children if it's in m_cursor_widget_stack
	}

#ifdef DEBUG

	void ContextManager::BuildWidgetListAtCursorPoint (const Point& cursor_point,
	        const AbstractWidget* parent)
	{
		if (parent) {
			for (std::set<AbstractWidget*>::iterator it =
			        parent->m_branches.begin(); it != parent->m_branches.end();
			        it++) {
				if ((*it)->contain(cursor_point)) {
					m_hover_deque->push_back(*it);
					BuildWidgetListAtCursorPoint(cursor_point, *it);
					break;	// if break or continue the loop?
				}
			}
		} else {
			m_hover_deque->clear();

			map<int, ContextLayer>::reverse_iterator layer_riter;
			set<AbstractWidget*>::iterator widget_iter;
			set<AbstractWidget*>* widget_set_p = 0;

			bool stop = false;

			for (layer_riter = m_layers.rbegin(); layer_riter != m_layers.rend();
			        layer_riter++) {
				widget_set_p = layer_riter->second.widgets;
				for (widget_iter = widget_set_p->begin(); widget_iter != widget_set_p->end();
				        widget_iter++) {
					if ((*widget_iter)->contain(cursor_point)) {
						m_hover_deque->push_back(*widget_iter);
						BuildWidgetListAtCursorPoint(cursor_point, *widget_iter);
						stop = true;
					}

					if (stop)
						break;
				}
				if (stop)
					break;
			}
		}
	}

	void ContextManager::RemoveWidgetFromHoverDeque(AbstractWidget* widget)
	{
		while(m_hover_deque->size()) {
			m_hover_deque->back()->m_flag.reset(AbstractWidget::WidgetFlagContextHoverList);

			if(m_hover_deque->back() == widget) {
				m_hover_deque->pop_back();
				break;
			}

			m_hover_deque->pop_back();
		}
	}

	void ContextManager::SetFocusedWidget (AbstractWidget* widget)
	{
		if(AbstractWidget::focused_widget) {
			AbstractWidget::focused_widget->m_flag.reset(AbstractWidget::WidgetFlagFocus);
		}

		AbstractWidget::focused_widget = widget;
		if(AbstractWidget::focused_widget) {
			AbstractWidget::focused_widget->m_flag.set(AbstractWidget::WidgetFlagFocus);
		}
	}

	void ContextManager::RefreshLayer (int layer)
	{
		map<int, ContextLayer>::iterator layer_iter;

		layer_iter = m_layers.find(layer);

		if(layer_iter != m_layers.end()) {
			m_layers[layer].refresh = true;
			refresh_once = true;
		}
	}

	void ContextManager::print ()
	{
		map<int, ContextLayer>::iterator layer_iter;
		set<AbstractWidget*>::iterator widget_iter;

		set<AbstractWidget*>* widget_set_p;
		std::cout << std::endl;

		std::cerr << "size of index map:" << m_index.size() << std::endl;
		std::cerr << "size of layer map:" << m_layers.size() << std::endl;

		for(layer_iter = m_layers.begin(); layer_iter != m_layers.end(); layer_iter++)
		{
			std::cerr << "Layer: " << layer_iter->first << std::endl;
			widget_set_p = layer_iter->second.widgets;
			for(widget_iter = widget_set_p->begin(); widget_iter != widget_set_p->end(); widget_iter++)
			{
				std::cerr << (*widget_iter)->name() << " ";
			}
			std::cerr << std::endl;
		}
	}

#endif

}
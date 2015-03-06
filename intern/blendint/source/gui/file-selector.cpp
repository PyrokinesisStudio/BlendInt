/*
 * This file is part of BlendInt (a Blender-like Interface Library in
 * OpenGL).
 *
 * BlendInt (a Blender-like Interface Library in OpenGL) is free software:
 * you can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * BlendInt (a Blender-like Interface Library in OpenGL) is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BlendInt.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Contributor(s): Freeman Zhang <zhanggyb@gmail.com>
 */

#include <boost/filesystem.hpp>

#include <glm/gtx/transform.hpp>

#include <gui/file-selector.hpp>
#include <gui/linear-layout.hpp>
#include <gui/splitter.hpp>
#include <gui/block.hpp>
#include <gui/folder-list.hpp>

#include <gui/close-button.hpp>

#include <gui/separator.hpp>
#include <gui/abstract-window.hpp>

namespace BlendInt {

	namespace fs = boost::filesystem;

	FileSelector::FileSelector ()
	: AbstractDialog(),
	  path_entry_(0),
	  file_entry_(0)
	{
		LinearLayout* button_layout = CreateButtons();

		//Frame* sidebar = CreateSideBarOnce();
		LinearLayout* area = CreateBrowserAreaOnce();

		LinearLayout* main_layout = new LinearLayout(Vertical);

		main_layout->AddWidget(button_layout);
		main_layout->AddWidget(area);

		main_layout->Resize(main_layout->GetPreferredSize());

		PushBackSubView(main_layout);

		set_round_type(RoundAll);
		set_round_radius(5.f);
		set_size(main_layout->size());
		set_refresh(true);
        EnableViewBuffer();

		projection_matrix_  = glm::ortho(
				0.f, (float)size().width(),
				0.f, (float)size().height(),
				100.f, -100.f);
		model_matrix_ = glm::mat3(1.f);

		std::vector<GLfloat> inner_verts;
		std::vector<GLfloat> outer_verts;

		if (AbstractWindow::theme->dialog().shaded) {
			GenerateRoundedVertices(Vertical,
					AbstractWindow::theme->dialog().shadetop,
					AbstractWindow::theme->dialog().shadedown,
					&inner_verts,
					&outer_verts);
		} else {
			GenerateRoundedVertices(&inner_verts, &outer_verts);
		}

		glGenVertexArrays(2, vao_);
		vbo_.generate();

		glBindVertexArray(vao_[0]);

		vbo_.bind(0);
		vbo_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);

		glEnableVertexAttribArray(AttributeCoord);
		glVertexAttribPointer(AttributeCoord, 3,
		GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(vao_[1]);

		vbo_.bind(1);
		vbo_.set_data(sizeof(GLfloat) * outer_verts.size(), &outer_verts[0]);
		glEnableVertexAttribArray(AttributeCoord);
		glVertexAttribPointer(AttributeCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

		shadow_.reset(new FrameShadow(size(), round_type(), round_radius()));

		std::string pwd =getenv("PWD");
		pwd.append("/");
		path_entry_->SetText(pwd);
		browser_->Open(getenv("PWD"));

		//events()->connect(dec->close_triggered(), this, &FileSelector::OnCloseButtonClicked);

		events()->connect(browser_->selected(), this, &FileSelector::OnFileSelect);
		//events()->connect(open_->clicked(), &opened_, &Cpp::Event<>::fire);
		//events()->connect(cancel_->clicked(), &canceled_, &Cpp::Event<>::fire);

		//events()->connect(cancel_->clicked(), this, &FileSelector::OnCancel);
	}

	FileSelector::~FileSelector ()
	{
        glDeleteVertexArrays(2, vao_);
	}

	void FileSelector::OnFileSelect ()
	{
		file_entry_->SetText(browser_->file_selected());
	}

	void FileSelector::PerformSizeUpdate (const SizeUpdateRequest& request)
	{
    	if(request.target() == this) {

    		set_size(*request.size());

    		projection_matrix_  = glm::ortho(
    				0.f,
    				0.f + (float)size().width(),
    				0.f,
    				0.f + (float)size().height(),
    				100.f, -100.f);

    		if(view_buffer()) {
    			view_buffer()->Resize(size());
    		}

    		shadow_->Resize(size());

    		std::vector<GLfloat> inner_verts;
    		std::vector<GLfloat> outer_verts;

    		if (AbstractWindow::theme->dialog().shaded) {
    			GenerateRoundedVertices(Vertical,
    					AbstractWindow::theme->dialog().shadetop,
    					AbstractWindow::theme->dialog().shadedown,
    					&inner_verts,
    					&outer_verts);
    		} else {
    			GenerateRoundedVertices(&inner_verts, &outer_verts);
    		}

    		vbo_.bind(0);
    		vbo_.set_data(sizeof(GLfloat) * inner_verts.size(), &inner_verts[0]);
    		vbo_.bind(1);
    		vbo_.set_data(sizeof(GLfloat) * outer_verts.size(), &outer_verts[0]);
    		vbo_.reset();

    		ResizeSubView(first_subview(), size());

    		RequestRedraw();

    	}

    	if(request.source() == this) {
    		ReportSizeUpdate(request);
    	}
	}

	bool FileSelector::PreDraw (AbstractWindow* context)
	{
		if(!visiable()) return false;

		context->register_active_frame(this);

		if(refresh() && view_buffer()) {
			RenderSubFramesToTexture(this,
					context,
					projection_matrix_,
					model_matrix_,
					view_buffer()->texture());
		}

		return true;
	}

	Response FileSelector::Draw (AbstractWindow* context)
	{
    	shadow_->Draw(position().x(), position().y());

		AbstractWindow::shaders->frame_inner_program()->use();

		glUniform2f(AbstractWindow::shaders->location(Shaders::FRAME_INNER_POSITION), position().x(), position().y());
		glUniform1i(AbstractWindow::shaders->location(Shaders::FRAME_INNER_GAMMA), 0);
		glUniform4fv(AbstractWindow::shaders->location(Shaders::FRAME_INNER_COLOR), 1, AbstractWindow::theme->dialog().inner.data());

		glBindVertexArray(vao_[0]);
		glDrawArrays(GL_TRIANGLE_FAN, 0, GetOutlineVertices(round_type()) + 2);

		if(view_buffer()) {

			AbstractWindow::shaders->frame_image_program()->use();

			glUniform2f(AbstractWindow::shaders->location(Shaders::FRAME_IMAGE_POSITION), position().x(), position().y());
			glUniform1i(AbstractWindow::shaders->location(Shaders::FRAME_IMAGE_TEXTURE), 0);
			glUniform1i(AbstractWindow::shaders->location(Shaders::FRAME_IMAGE_GAMMA), 0);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			view_buffer()->Draw(0, 0);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		} else {

            glViewport(position().x(), position().y(), size().width(), size().height());

            AbstractWindow::shaders->SetWidgetProjectionMatrix(projection_matrix_);
            AbstractWindow::shaders->SetWidgetModelMatrix(model_matrix_);

			DrawSubViewsOnce(context);

			glViewport(0, 0, context->size().width(), context->size().height());

		}

		AbstractWindow::shaders->frame_outer_program()->use();

		glUniform2f(AbstractWindow::shaders->location(Shaders::FRAME_OUTER_POSITION), position().x(), position().y());
		glUniform4fv(AbstractWindow::shaders->location(Shaders::FRAME_OUTER_COLOR), 1, AbstractWindow::theme->dialog().outline.data());

		glBindVertexArray(vao_[1]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, GetOutlineVertices(round_type()) * 2 + 2);

        return Finish;
	}

	void FileSelector::OnClose(AbstractButton* sender)
	{
		AbstractView* super = superview();
		delete this;

		super->RequestRedraw();
	}

	void FileSelector::OnOpenParent (AbstractButton* sender)
	{
		if(browser_->OpenParent()) {
			path_entry_->SetText(browser_->pathname());
		}
	}

	void FileSelector::OnGoBackward (AbstractButton* sender)
	{
		if(browser_->GoBackward()) {
			path_entry_->SetText(browser_->pathname());
		}
	}

	void FileSelector::OnGoForward (AbstractButton* sender)
	{
		if(browser_->GoForward()) {
			path_entry_->SetText(browser_->pathname());
		}
	}

	void FileSelector::OnReload (AbstractButton* sender)
	{
		browser_->Open(browser_->pathname());
	}

	void FileSelector::OnOpen(AbstractButton* sender)
	{
		// TODO: check directory mode or file mode
		if(!browser_->file_selected().empty()) {

			std::string file = ConvertFromString(browser_->file_selected());

			if(browser_->pathname() != "/") {
				file = browser_->pathname() + "/" + file;
			} else {
				file = browser_->pathname() + file;
			}

			fs::path p(file);
			if(fs::is_directory(p)) {
				p = fs::absolute(p);
				browser_->Open(p.native());
				path_entry_->SetText(browser_->pathname());

				return;
			}
		}

		AbstractView* super = superview();
		fire_applied_event();

		delete this;
		super->RequestRedraw();
	}

	LinearLayout* FileSelector::CreateButtons()
	{
		LinearLayout* hlayout = Manage(new LinearLayout);
		DBG_SET_NAME(hlayout, "FileSelectorDecorationLayout");
		hlayout->SetMargin(Margin(0, 0, 0, 0));

		// create close button
		CloseButton* close_button = Manage(new CloseButton);

		// directory control group
		Block* block1 = Manage(new Block);

		Button* btn_back = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::BACK)));
		Button* btn_forward = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::FORWARD)));
		Button* btn_up = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::FILE_PARENT)));
		Button* btn_reload = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::FILE_REFRESH)));

		block1->AddWidget(btn_back);
		block1->AddWidget(btn_forward);
		block1->AddWidget(btn_up);
		block1->AddWidget(btn_reload);

		block1->Resize(block1->GetPreferredSize());

		// create new
		Button* btn_new = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::NEWFOLDER), "Create New Directory"));

		// display mode
		Block* block2 = Manage(new Block);

		Button* btn_short_list = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::SHORTDISPLAY)));
		Button* btn_detail_list = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::LONGDISPLAY)));
		Button* btn_thumbnail = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::IMGDISPLAY)));

		block2->AddWidget(btn_short_list);
		block2->AddWidget(btn_detail_list);
		block2->AddWidget(btn_thumbnail);

		Block* block3 = Manage(new Block);

		Button* btn_sort_alpha = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::SORTALPHA)));
		Button* btn_sort_ext = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::SORTBYEXT)));
		Button* btn_sort_time = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::SORTTIME)));
		Button* btn_sort_size = Manage(new Button(AbstractWindow::icons->icon_16x16(Icons::SORTSIZE)));

		block3->AddWidget(btn_sort_alpha);
		block3->AddWidget(btn_sort_ext);
		block3->AddWidget(btn_sort_time);
		block3->AddWidget(btn_sort_size);

		Button* open = Manage(new Button(String("Open")));
		DBG_SET_NAME(open, "Open Button");

		Separator* separator1 = Manage(new Separator);
		Separator* separator2 = Manage(new Separator);
		Separator* separator3 = Manage(new Separator(true));

		hlayout->AddWidget(close_button);
		hlayout->AddWidget(separator1);
		hlayout->AddWidget(block1);
		hlayout->AddWidget(separator2);
		hlayout->AddWidget(btn_new);
		hlayout->AddWidget(block2);
		hlayout->AddWidget(block3);
		hlayout->AddWidget(separator3);
		hlayout->AddWidget(open);

		hlayout->Resize(hlayout->GetPreferredSize());

		events()->connect(close_button->clicked(), this, &FileSelector::OnClose);
		events()->connect(btn_back->clicked(), this, &FileSelector::OnGoBackward);
		events()->connect(btn_forward->clicked(), this, &FileSelector::OnGoForward);
		events()->connect(btn_up->clicked(), this, &FileSelector::OnOpenParent);
		events()->connect(btn_reload->clicked(), this, &FileSelector::OnReload);
		events()->connect(open->clicked(), this, &FileSelector::OnOpen);

		return hlayout;
	}

	LinearLayout* FileSelector::CreateBrowserAreaOnce()
	{
		LinearLayout* vbox = Manage(new LinearLayout(Vertical));
		vbox->SetMargin(Margin(0, 0, 0, 0));
		DBG_SET_NAME(vbox, "VBox in Broser Area");
		vbox->SetSpace(2);

		path_entry_ = Manage(new TextEntry);
		DBG_SET_NAME(path_entry_, "Path Entry");
		path_entry_->SetRoundType(RoundAll);

		file_entry_ = Manage(new TextEntry);
		DBG_SET_NAME(file_entry_, "File Entry");
		file_entry_->SetRoundType(RoundAll);

		browser_ = Manage(new FileBrowser);
		DBG_SET_NAME(browser_, "FileBrowser");

		vbox->AddWidget(path_entry_);
		vbox->AddWidget(file_entry_);
		vbox->AddWidget(browser_);

		return vbox;
	}

	/*
	Frame* FileSelector::CreateSideBarOnce ()
	{
		Frame* toolbox = Manage(new Frame);
		DBG_SET_NAME(toolbox, "SideBar");
		toolbox->SetMargin(2, 2, 2, 2);

		Expander* exp1 = CreateSystemDevicesOnce();
		Expander* exp2 = CreateSystemBookmarksOnce();

		toolbox->Append(exp1);
		toolbox->Append(exp2);

		return toolbox;
	}
	*/

	Expander* FileSelector::CreateSystemDevicesOnce ()
	{
		Expander* expander = Manage(new Expander("System"));
		DBG_SET_NAME(expander, "System Expander");

		FolderList* system_folders = Manage(new FolderList);
		DBG_SET_NAME(system_folders, "System Folders");

		expander->AddWidget(system_folders);

		return expander;
	}

	Expander* FileSelector::CreateSystemBookmarksOnce ()
	{
		Expander* expander = Manage(new Expander("System Bookmarks"));
		DBG_SET_NAME(expander, "System Bookmarks Expander");

		FolderList* system_bookmark = Manage(new FolderList);
		DBG_SET_NAME(system_bookmark, "System Bookmarks");

		expander->AddWidget(system_bookmark);

		return expander;
	}

}

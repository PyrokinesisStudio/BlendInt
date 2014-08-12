/**
 * Main Layout
 */

#include <BlendInt/Core/String.hpp>
#include <BlendInt/Gui/Menu.hpp>
#include <BlendInt/Stock/Icons.hpp>
#include <BlendInt/Gui/Button.hpp>
#include <BlendInt/Gui/Splitter.hpp>

#include <BlendInt/Gui/VBlockLayout.hpp>
#include <BlendInt/Gui/ImageView.hpp>
#include <BlendInt/Gui/ComboBox.hpp>
#include <BlendInt/Gui/HLayout.hpp>

#include "MainLayout.hpp"

MainLayout::MainLayout ()
	: m_menubar(0), m_toolbar(0), m_scene(0), m_file_input(0), m_file_button(0)
{
	InitOnce();
}

MainLayout::~MainLayout ()
{

}

void MainLayout::InitOnce ()
{
	using namespace BI;

	set_margin(0, 0, 0, 0);
	set_space(1);

	m_menubar = CreateMenuBar();
	m_toolbar = CreateToolBar();

    Splitter* splitter = Manage(new Splitter);
    splitter->SetMargin(0, 0, 0, 0);

    ToolBox* tbox = CreateToolBox();

    Tab* tab = CreateTab();

    ToolBar* bottom = CreateBottomBar();

    splitter->PushBack(tab);
    splitter->PushBack(tbox);

	PushBack(m_menubar);
	PushBack(m_toolbar);
    PushBack(splitter);
    PushBack(bottom);

	events()->connect(m_open->clicked(), this, &MainLayout::OnOpenClick);
}

void MainLayout::OnOpenClick()
{
}

void MainLayout::OnResize (AbstractWidget* context, int type)
{
	Resize(context->size());
}

BI::ToolBar* MainLayout::CreateToolBar()
{
	using namespace BI;

	ToolBar* toolbar = Manage(new ToolBar);

	m_input = Manage(new TextEntry);
	m_open = Manage(new ToolButton);

	toolbar->PushBack(m_input);
	toolbar->PushBack(m_open);

	return toolbar;
}

BI::MenuBar* MainLayout::CreateMenuBar()
{
	using namespace BI;

	MenuBar* menubar = Manage(new MenuBar);

	RefPtr<Menu> file_menu(new Menu);

    file_menu->SetRoundType(RoundAll);
    file_menu->AddAction(Stock::Icons::instance->icon_check(), "MenuItem1", "Ctrl + 1");
    file_menu->AddAction("MenuItem2", "Ctrl + 1");
    file_menu->AddAction("MenuItem3", "Ctrl + 1");
    file_menu->AddAction("MenuItem4", "Ctrl + 1");
    file_menu->AddAction("MenuItem5");

	menubar->PushBack(String("File"), file_menu);

	return menubar;
}

BI::ToolBox* MainLayout::CreateToolBox()
{
	using namespace BI;

	ToolBox* toolbox = Manage(new ToolBox);

	Expander* expander = CreateExpander();
	toolbox->PushBack(expander);

	return toolbox;
}

BI::Expander* MainLayout::CreateExpander()
{
	using namespace BI;

	Expander* expander = Manage(new Expander("Transform"));

	Button* btn1 = Manage(new Button("Translate"));
	Button* btn2 = Manage(new Button("Rotate"));
	Button* btn3 = Manage(new Button("Scale"));

	VBlock* vblock = Manage(new VBlock);
	vblock->PushBack(btn1);
	vblock->PushBack(btn2);
	vblock->PushBack(btn3);

	expander->Setup(vblock);

	return expander;
}

BI::Tab* MainLayout::CreateTab ()
{
	using namespace BI;

	Tab* tab = Manage(new Tab);
	tab->SetMargin(0, 0, 0, 0);

    m_scene = Manage(new Viewport3D);
    ImageView* iv = Manage(new ImageView);

    tab->Add("3D View", m_scene);
    tab->Add("Image View", iv);

	return tab;
}

BI::ToolBar* MainLayout::CreateBottomBar ()
{
	using namespace BI;

	ToolBar* toolbar = Manage(new ToolBar);

	ComboBox* combo = Manage(new ComboBox);

	HBox* box = Manage(new HBox);
	box->SetMargin(0, 0, 0, 0);
	box->SetSpace(-1);
	m_file_input = Manage(new TextEntry);
	m_file_button = Manage(new FileButton);
	m_file_input->SetRoundType(RoundTopLeft | RoundBottomLeft);
	m_file_button->SetRoundType(RoundTopRight | RoundBottomRight);
	box->PushBack(m_file_input);
	box->PushBack(m_file_button);

	toolbar->SetMargin(2, 2, 2, 2);
	toolbar->PushBack(combo);
	toolbar->PushBack(box);

	events()->connect(m_file_button->file_opened(), this, &MainLayout::OnFileChanged);

	return toolbar;
}

void MainLayout::OnFileChanged ()
{
	m_file_input->SetText(m_file_button->file());
}

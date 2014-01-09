/**
 * BlendInt demo
 */

#include <iostream>

#include <Cpp/Events.hpp>
#include <BlendInt/Interface.hpp>
#include <BlendInt/ContextManager.hpp>

#include <BlendInt/ToggleButton.hpp>
#include <BlendInt/VerticalLayout.hpp>
#include <BlendInt/HorizontalLayout.hpp>

#include <BlendInt/Types.hpp>
#include <BlendInt/Button.hpp>
#include <BlendInt/Label.hpp>
#include <BlendInt/ScrollWidget.hpp>
#include <BlendInt/Slider.hpp>
#include <BlendInt/Frame.hpp>
#include <BlendInt/VertexIcon.hpp>
#include <BlendInt/TableLayout.hpp>
#include <BlendInt/ScrollBar.hpp>
#include <BlendInt/VertexIcon.hpp>
#include <BlendInt/ScrollView.hpp>
#include <BlendInt/PopupWidget.hpp>
#include <BlendInt/Menu.hpp>
#include <BlendInt/RoundWidget.hpp>
#include <BlendInt/ImageView.hpp>
#include <BlendInt/TabFrame.hpp>
#include <BlendInt/MenuItemBin.hpp>
#include <BlendInt/StockIcon.hpp>
#include <BlendInt/GLTexture2D.hpp>
#include <BlendInt/TextEntry.hpp>

#include "Window.hpp"

#include "DemoFrame.hpp"

using namespace BlendInt;
using namespace std;

int main(int argc, char* argv[]) {
	BLENDINT_EVENTS_INIT_ONCE_IN_MAIN;

	Init ();
	GLFWwindow* window = CreateWindow("MenuTest1 - Click1");

	// TODO: add test code here
	TextEntry* widget = new TextEntry;
	widget->set_name("TextEntry1");
	widget->SetFont(Font("Droid Sans"));
	widget->SetPosition(200, 200);

	widget->Register();
	widget->show();

	TextEntry* widget2 = new TextEntry;
	widget2->set_name("TextEntry2");
	widget2->SetFont(Font("Droid Sans"));
	widget2->SetPosition(400, 200);

	widget2->Register();
	widget2->show();

	Button* button = new Button;
	button->set_name("button1");
	button->SetPosition(200, 300);
	button->Register();
	button->show();

	RunLoop(window);

	Terminate();

}


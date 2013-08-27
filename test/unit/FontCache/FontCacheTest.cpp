// cpp

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include <BIL/FontCache.hpp>
#include <BIL/FontConfig.hpp>
#include <BIL/String.hpp>

#include "FontCacheTest.h"

using namespace BIL;
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(FontCacheTest);

FontCacheTest::FontCacheTest ()
{

}

FontCacheTest::~FontCacheTest ()
{

}

void FontCacheTest::setUp ()
{
	int result = glfwInit();

	if (result != GL_TRUE) {
		CPPUNIT_FAIL("Cannot initialize glfw\n");
	}

	bool ret = false;

	FontConfig::instance();

	ret = FontConfig::getService()->initialize();
	if(!ret) {
		CPPUNIT_FAIL("Cannot initialize FontConfig service\n");
	}

	ret = FontConfig::getService()->loadDefaultFontToMem(); // load default font to memory
	if (!ret) {
		// TODO: stop and show failure of this TestFixture
		CPPUNIT_FAIL("Fail to load default font to memory");
	}
}

void FontCacheTest::tearDown ()
{
	FontConfig::release();
	glfwTerminate();
}

void FontCacheTest::create1 ()
{
	FontCache* cache = FontCache::create();

	FontCache::release();

	CPPUNIT_ASSERT(cache != NULL);
}

void FontCacheTest::create2 ()
{
	cout << endl << "reach here!" << endl;
	FontCache* cache = FontCache::create(Font("Droid Sans"));

	cout << "and reach here!" << endl;

	FontCache::release(Font("Droid Sans"));
	cout << "and finally reach here!" << endl;

	CPPUNIT_ASSERT(cache != NULL);
}

void FontCacheTest::create3 ()
{
	FontCache::create(Font("Droid Sans"));

	bool result = FontCache::release(Font("Sans"));

	FontCache::release(Font("Droid Sans"));

	CPPUNIT_ASSERT(!result);
}

void FontCacheTest::create4 ()
{
	FontCache* cache = FontCache::create(Font("Droid Sans Mono"));

	if (cache == NULL) {
		CPPUNIT_FAIL ("Cannot create cache for Droid Sans Mono\n");
	}

	bool result = cache->Initialize();

	if (result) {
		Glyph* glyph = cache->query('A');
		if(glyph == NULL) {
			result = false;
		} else {
			cout << "Glyph Index: " << glyph->glyph_index() << endl;
		}
	}

	FontCache::release (Font("Droid Sans Mono"));

	CPPUNIT_ASSERT(result);
}

void FontCacheTest::check1 ()
{
	FontCache* cache = FontCache::create(Font("Droid Sans Mono"));

	if(cache == NULL)
		CPPUNIT_FAIL ("Cannot create cache for Droid Sans Mono\n");

	bool result = cache->Initialize();

	if(result) {
		cache->query(L'仁', true);
		cache->query(L'义', true);
		cache->query(L'礼', true);
		cache->query(L'智', true);
		cache->query(L'信', true);


		cache->query(L'仁');
		cache->query(L'仁');

#ifdef DEBUG
		cache->printcount();

		FontCache::list ();

#endif
	}

	FontCache::release (Font("Droid Sans Mono"));

#ifdef DEBUG
	FontCache::list();
#endif

	CPPUNIT_ASSERT (result);
}

void FontCacheTest::check2 ()
{
	FontCache* cache = FontCache::create(Font("Droid Sans Mono"));
	Glyph* glyph = NULL;

	if(cache == NULL)
		CPPUNIT_FAIL ("Cannot create cache for Droid Sans Mono\n");

	bool result = cache->Initialize();

	if(result) {
#ifdef DEBUG
		cache->printcount();
		FontCache::list();
#endif
		glyph = cache->query('A');
	}

	FontCache::release (Font("Droid Sans Mono"));

	CPPUNIT_ASSERT (glyph != NULL && result);
}

void FontCacheTest::check3 ()
{
	FontCache* cache = FontCache::create();
	//Glyph* glyph = NULL;

	if(cache == NULL)
		CPPUNIT_FAIL ("Cannot create cache for Droid Sans Mono\n");

	bool result = cache->Initialize();

	if(result) {
#ifdef DEBUG
		cache->printcount();
		FontCache::list();
#endif
		//glyph = cache->query('A');
	}

	FontCache::release ();

	CPPUNIT_ASSERT (result);
}

void FontCacheTest::check4 ()
{
	FontCache::create(Font("Sans"));
	FontCache::create(Font("Droid Sans"));
	FontCache::create(Font("Droid Serif"));
	FontCache::create(Font("Droid Sans Mono"));
	FontCache::create(Font("Droid Sans Thai"));

#ifdef DEBUG
		FontCache::list();
#endif

	FontCache::releaseAll ();
#ifdef DEBUG
		FontCache::list();
#endif

	CPPUNIT_ASSERT (true);
}

void FontCacheTest::check5 ()
{
	FontCache::setMaxCaches(6);
	FontCache::create(Font("Droid Sans"));
	FontCache::create(Font("Droid Serif"));
	FontCache::create(Font("Droid Sans Mono"));
	FontCache::create(Font("Droid Sans Thai"));
	FontCache::create(Font("Droid Sans Hebrew"));
	FontCache::create(Font("Droid Sans Fallback"));
	FontCache::create(Font("Droid Sans Japanese"));

#ifdef DEBUG
		FontCache::list();
#endif

	FontCache::releaseAll ();
#ifdef DEBUG
		FontCache::list();
#endif

		FontCache::setMaxCaches(32);

	CPPUNIT_ASSERT (true);
}

void FontCacheTest::check6 ()
{
	FontCache* cache = FontCache::create();
	Glyph* glyph = NULL;

	if(cache == NULL)
		CPPUNIT_FAIL ("Cannot create cache for default font\n");

	bool result = cache->Initialize();

	if(result) {
		for(int i = 0; i < 10; i++)
		{
			glyph = cache->query(L'仁');
		}
		for(int i = 0; i < 5; i++)
		{
			glyph = cache->query(L'义');
		}
		for(int i = 0; i < 20; i++)
		{
			glyph = cache->query(L'智');
		}

#ifdef DEBUG
		cache->printcount();
#endif

	}

	FontCache::releaseAll ();

	CPPUNIT_ASSERT (true);
}

void FontCacheTest::check7 ()
{

	FontCache::setMaxCaches(6);
	FontCache* cache1 = FontCache::create(Font("Droid Sans"));
	FontCache* cache2 = FontCache::create(Font("Droid Sans"));

#ifdef DEBUG
		FontCache::list();
#endif

	FontCache::releaseAll ();

#ifdef DEBUG
		FontCache::list();
#endif

		FontCache::setMaxCaches(32);

	CPPUNIT_ASSERT (cache1 == cache2);
}

void FontCacheTest::check8 ()
{

	FontCache::setMaxCaches(6);
	FontCache* cache1 = FontCache::create(Font("Droid Sans"));
	bool result1 = cache1->Initialize();
	FontCache* cache2 = FontCache::create(Font("Droid Sans", 12));
	bool result2 = cache2->Initialize();

#ifdef DEBUG
		FontCache::list();
#endif

	FontCache::releaseAll ();

#ifdef DEBUG
		FontCache::list();
#endif

		FontCache::setMaxCaches(32);

	CPPUNIT_ASSERT ((cache1 != cache2) && result1 && result2);
}

void FontCacheTest::show1()
{
	GLFWwindow * win = glfwCreateWindow(640, 480, "FontCache Test", NULL,
	        NULL);

	if (win == NULL) {
		CPPUNIT_FAIL("Cannot create glfw window\n");
	}

	glfwMakeContextCurrent(win);

	// Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initilize GLEW" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	FontCache* cache = FontCache::create(Font("Droid Sans Fallback", 24));
	Glyph* glyph = NULL;

	if(cache == NULL)
		CPPUNIT_FAIL ("Cannot create cache for default font\n");

	bool result = cache->Initialize();
	if(!result) {
		CPPUNIT_FAIL("Cannot initialize font cache\n");
	}

	glyph = cache->query(L'信');

	while (!glfwWindowShouldClose(win)) {

		int width, height;

		glfwGetWindowSize(win, &width, &height);

		glClearColor(0.40, 0.40, 0.45, 1.00);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor4f(1.00, 1.00, 1.00, 1.00);

		// enable anti-alias
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);

		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.f, (float) width, 0.f, (float) height, 100.f, -100.f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Test buffer render
		glyph->Render();

		glfwSwapBuffers(win);
		glfwPollEvents();
	}

	FontCache::releaseAll();

	CPPUNIT_ASSERT(true);
}

void FontCacheTest::show_multiple_cache1()
{
	GLFWwindow * win = glfwCreateWindow(1024, 640, "FontCache Test", NULL,
	        NULL);

	if (win == NULL) {
		CPPUNIT_FAIL("Cannot create glfw window\n");
	}

	glfwMakeContextCurrent(win);

	// Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initilize GLEW" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	FontCache* cache1 = FontCache::create(Font("Lucida Grande", 48));
	cache1->Initialize();

	//FontCache* cache2 = FontCache::create(Font("Sans", 48));
	FontCache* cache2 = FontCache::create(Font("Droid Sans Mono", 48));
	cache2->Initialize();

	FontCache* cache3 = FontCache::create(Font("DejaVu Serif", 48));
	cache3->Initialize();

	//FontCache* cache4 = FontCache::create(Font("STXingkai", 48));
	FontCache* cache4 = FontCache::create(Font("Bitstream Vera Sans", 48));
	//cache4->Initialize();

	String str1("Hello World! (cache1)");
	//String str2(L"花间一壶酒，独酌无相亲。");
	String str2("Hello World! (cache2)");
	String str3("Hello World! (cache3)");
	//String str4(L"举杯邀明月，对影成三人。");
	String str4("Hello World! (cache4)");
	String::const_iterator it;

#ifdef DEBUG
	FontCache::list();
#endif

	Glyph* glyph = NULL;

	while (!glfwWindowShouldClose(win)) {

		int width, height;

		glfwGetWindowSize(win, &width, &height);

		glClearColor(0.40, 0.40, 0.45, 1.00);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor4f(1.00, 1.00, 1.00, 1.00);

		// enable anti-alias
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POLYGON_SMOOTH);

		glViewport(0, 0, width, height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.f, (float) width, 0.f, (float) height, 100.f, -100.f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		// Test buffer render
		glPushMatrix();
		glTranslatef(100.0f, 100.0f, 0.0f);
		for (it = str1.begin(); it != str1.end(); it++)
		{
			glyph = cache1->query(*it);
			glyph->Render();
			glTranslatef(glyph->metrics().horiAdvance, 0, 0);
		}
		glPopMatrix();

		glPushMatrix();
		glTranslatef(100.0f, 200.0f, 0.0f);
		for (it = str2.begin(); it != str2.end(); it++)
		{
			glyph = cache2->query(*it);
			glyph->Render();
			glTranslatef(glyph->metrics().horiAdvance, 0, 0);
		}
		glPopMatrix();

		glPushMatrix();
		glTranslatef(100.0f, 300.0f, 0.0f);
		for (it = str3.begin(); it != str3.end(); it++)
		{
			glyph = cache3->query(*it);
			glyph->Render();
			glTranslatef(glyph->metrics().horiAdvance, 0, 0);
		}
		glPopMatrix();

		glPushMatrix();
		glTranslatef(100.0f, 400.0f, 0.0f);
		for (it = str4.begin(); it != str4.end(); it++)
		{
			glyph = cache4->query(*it);
			glyph->Render();
			glTranslatef(glyph->metrics().horiAdvance, 0, 0);
		}
		glPopMatrix();

		glfwSwapBuffers(win);
		glfwPollEvents();
	}

	FontCache::releaseAll();

	CPPUNIT_ASSERT(true);
}

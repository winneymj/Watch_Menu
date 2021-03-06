/*********************************************************************
 This is a library for SHARP Memory Display

 Written by Mark Winney.
 BSD license, check license.txt for more information
 All text above, and the splash screen must be included in any redistribution
 *********************************************************************/

#ifdef __AVR__
  #include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
 #include <pgmspace.h>
#else
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#if !defined(__ARM_ARCH) && !defined(ENERGIA) && !defined(ESP8266) && !defined(ESP32) && !defined(__arc__)
 #include <util/delay.h>
#endif
#include <stdlib.h>

// Many (but maybe not all) non-AVR board installs define macros
// for compatibility with existing PROGMEM-reading AVR code.
// Do our own checks and defines here for good measure...

#ifndef pgm_read_byte
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
 #define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
 #define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

#include "Adafruit_SharpMem.h"
#include "watch_menu.h"

#define NOINVERT	false
#define YPOS		64

extern const uint8_t selectbar_top[];
extern const uint8_t selectbar_bottom[];
extern const uint8_t menu_default[];
extern const uint8_t selectbar_topWidthPixels;

WatchMenu::WatchMenu (Adafruit_SharpMem& display) : m_display (display), m_inverted(false), menus(NULL)
{
}

void WatchMenu::setDownFunc(pFunc func)
{
	menus[menu_selected]->downFunc = func;
}

void WatchMenu::setUpFunc(pFunc func)
{
	menus[menu_selected]->upFunc = func;
}

void WatchMenu::setDrawFunc(pFunc func)
{
	menus[menu_selected]->drawFunc = func;
}

bool WatchMenu::menuDown(void)
{
  // See if the standard down option has been overridden and call the method
  if (NULL != menus[menu_selected]->downFunc)
  {
	  menus[menu_selected]->downFunc();
	  return true;
  }
  return false;
}

bool WatchMenu::menuUp()
{
  if (NULL != menus[menu_selected]->upFunc)
  {
	  menus[menu_selected]->upFunc();
	  return true;
  }
  return false;
}

void WatchMenu::downOption (void)
{
	while (true)
	{
		menus[menu_selected]->option_selected++;
		uint8_t sel = menus[menu_selected]->option_selected;

		if (sel >= menus[menu_selected]->num_options)
		{
			menus[menu_selected]->option_selected = 0;
		}

		// See if an option has been defined.  Exit if got one.
		if (NULL != menus[menu_selected]->options[menus[menu_selected]->option_selected])
		{
			return;
		}
	}
}

void WatchMenu::upOption ()
{
	while (true)
	{
		menus[menu_selected]->option_selected--;
		uint8_t sel = menus[menu_selected]->option_selected;

		if (sel >= menus[menu_selected]->num_options)
		{
			menus[menu_selected]->option_selected = menus[menu_selected]->num_options - 1;
		}
		// See if an option has been defined.  Exit if got one.
		if (NULL != menus[menu_selected]->options[menus[menu_selected]->option_selected])
		{
			return;
		}
	}
}

bool WatchMenu::selectOption (void)
{
  // Move to the next menu, assuming the option selected is a menu
  int8_t optSel = menus[menu_selected]->option_selected;
  pFunc funct = menus[menu_selected]->options[optSel]->func;
  bool subMenu = (funct == NULL);

  if (subMenu == true)
  {
    // Get the index to the sub menu
    int8_t menuIndex = menus[menu_selected]->options[optSel]->menu_index;

    // See if this is the exit option (assumed last option)
    if (optSel == (menus[menu_selected]->num_options - 1))
    {
		// Reset sub menu selected option to zero before we exit, so when
		// we come back in we are back at start
		menus[menu_selected]->option_selected = 0;

		// Reset animation X
		menus[menu_selected]->animX = m_display.width() / 2;

		// Go back to previous menu
		menu_selected = menus[menu_selected]->prev_menu;
    }
    else
    {
      // Store where you came from into the new menu so can get back when exit menu
      uint8_t tempMenu = menu_selected;

      // Change to the new menu
      menu_selected = menuIndex;

      // Store where you came from into the new menu so can get back when exit menu
      menus[menu_selected]->prev_menu = tempMenu;
    }
  }
  else
  {
    // Must call the function setup
    funct ();
  }
  return true;
}

void WatchMenu::initMenu(uint8_t num)
{
	num_menus = num;
	menus = new s_menu*[num]; // Allocate space for the menus.  Array of pointers to menus
	
	for (int index = 0; index < num; index++)
	{
		menus[index] = NULL;
	}
	
	menu_selected = 0;
	m_font = NULL;
	// Default font to default 5x7 builtin
	m_fontWidth = 5;
	m_fontHeight = 7;

}

void WatchMenu::createMenu (int8_t index, int8_t num_options, const char *name, int8_t menu_type)
{
	createMenu (index, num_options, name, menu_type, NULL, NULL);
}

// Create a menu by allocating space
void WatchMenu::createMenu (int8_t index, int8_t num_options, const char *name, int8_t menu_type, pFunc downFunc, pFunc upFunc)
{
	if (menus[index] == NULL)
	{
		menus[index] = new s_menu; // allocate space for the menu
	}
	strcpy_P(menus[index]->name, name);
	menus[index]->options = new s_option*[num_options]; // Allocate array of pointers to options
	menus[index]->num_options = num_options;
	menus[index]->option_selected = 0;
	menus[index]->type = menu_type;
	menus[index]->downFunc = downFunc;
	menus[index]->upFunc = upFunc;
	menus[index]->drawFunc = NULL;

	for (int opt_index = 0; opt_index < num_options; opt_index++)
	{
		menus[index]->options[opt_index] = NULL;
		menus[index]->prev_menu = NULL;
	}

	// Set the start animination point...number of icons - 1 * icon width
	menus[index]->animX = m_display.width() / 2;
}

void WatchMenu::createOption (int8_t menu_index, int8_t opt_index,
	int16_t invert_start, int16_t invert_length, const char *name,
	const uint8_t *icon, pFunc actionFunc)
{
	createOption (menu_index, opt_index, name, icon, actionFunc);
	menus[menu_index]->options[opt_index]->invert_start = invert_start;
	menus[menu_index]->options[opt_index]->invert_length = invert_length;
}

void WatchMenu::createOption (int8_t menu_index, int8_t opt_index, const char *name,
			const uint8_t *icon, pFunc actionFunc)
{
	if (menus[menu_index]->options[opt_index] == NULL)
	{
		menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
	}

	menus[menu_index]->options[opt_index]->func = actionFunc;
	menus[menu_index]->options[opt_index]->icon = icon;
	strcpy_P(menus[menu_index]->options[opt_index]->name, name);
	menus[menu_index]->options[opt_index]->menu_index = -1;
	menus[menu_index]->options[opt_index]->invert_start = -1;
	menus[menu_index]->options[opt_index]->invert_length = 0;
}

void WatchMenu::createOption (int8_t menu_index, int8_t opt_index, const char *name,
			const uint8_t *icon, uint8_t prev_menu_index)
{
	if (menus[menu_index]->options[opt_index] == NULL)
	{
		menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
	}
	menus[menu_index]->options[opt_index]->func = NULL;
	menus[menu_index]->options[opt_index]->icon = icon;
	strcpy_P(menus[menu_index]->options[opt_index]->name, name);
	menus[menu_index]->options[opt_index]->menu_index = prev_menu_index;
	menus[menu_index]->options[opt_index]->invert_start = -1;
	menus[menu_index]->options[opt_index]->invert_length = 0;
}

void WatchMenu::createOption (int8_t menu_index, int8_t opt_index, pFunc actionFunc,
			uint8_t prev_menu_index)
{
	if (menus[menu_index]->options[opt_index] == NULL)
	{
		menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
	}
	menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
	menus[menu_index]->options[opt_index]->func = actionFunc;
	menus[menu_index]->options[opt_index]->menu_index = prev_menu_index;
	menus[menu_index]->options[opt_index]->invert_start = -1;
	menus[menu_index]->options[opt_index]->invert_length = 0;
}

void WatchMenu::createOption (int8_t menu_index, int8_t opt_index, const char *name,
			uint8_t prev_menu_index)
{
	if (menus[menu_index]->options[opt_index] == NULL)
	{
		menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
	}
	menus[menu_index]->options[opt_index] = new s_option; // allocate space for the option
	strcpy_P(menus[menu_index]->options[opt_index]->name, name);
	menus[menu_index]->options[opt_index]->menu_index = prev_menu_index;
	menus[menu_index]->options[opt_index]->invert_start = -1;
	menus[menu_index]->options[opt_index]->invert_length = 0;
}

void WatchMenu::menu_drawStr()
{
	const int16_t displayWidth = m_display.width();
	const int16_t displayHeight = m_display.height();

	// Copy the data from PROGMEM...max 20 chars
	char tmpStr[20] = { 0 };
	strcpy_P (tmpStr, menus[menu_selected]->name);
	// Calculate Y position using the height of the font.

	uint8_t h = fontHeight() + (fontHeight() / 2);  // Add some spacing
	drawCentreString(tmpStr, displayWidth / 2, YPOS + h, textSize);

	byte count = menus[menu_selected]->num_options;
	byte opt = 0;
	// The last option is assumed to be exit...so draw on same line to the bottom
	// right of the display
	for(; opt < count - 1; opt++)
	{
		char tmpStr[20] = { 0 };
		if (NULL != menus[menu_selected]->options[opt])
		{
			if(opt == menus[menu_selected]->option_selected)
			{
				drawString(">", 0, YPOS + (h * (opt + 2)));
			}
			const char *str = menus[menu_selected]->options[opt]->name;
			strcpy_P (tmpStr, str);
		}

		// See about inverting some text
		int16_t invStart = menus[menu_selected]->options[opt]->invert_start;
		int16_t invLen = menus[menu_selected]->options[opt]->invert_length;


		if (invStart >= 0)
		{
			// Split the string into 3 parts around the invertion
			char tmpStartStr[20] = { 0 };
			strncpy_P(tmpStartStr, tmpStr, invStart);
			char tmpInvertStr[20] = { 0 };
			strncpy_P(tmpInvertStr, tmpStr + invStart, invLen);
			char tmpEndStr[20] = { 0 };
			strcpy_P(tmpEndStr, tmpStr + invStart + invLen);
			int16_t ypos = YPOS + (h * (opt + 2));
			int16_t xpos = fontWidth();

			drawString(tmpStartStr, xpos, ypos);
			xpos += fontWidth() * strlen(tmpStartStr);
			// Display background
        	m_display.fillRect(xpos, ypos - (fontHeight() +  1), fontWidth() * invLen, fontHeight() +  3, m_inverted ? WHITE : BLACK);

			// invert the text.
			m_inverted = !m_inverted;
			drawString(tmpInvertStr, xpos, ypos);
			m_inverted = !m_inverted;
			xpos += fontWidth() * strlen(tmpInvertStr);
			drawString(tmpEndStr, xpos, ypos);
		}
		else
		{
			drawString(tmpStr, fontWidth(), YPOS + (h * (opt + 2)));
		}
	}

	// Display the exit at right side of the screen
	const char *str = menus[menu_selected]->options[opt]->name;
	uint8_t exitStrLen = strlen(str) + 2; // Add 1 for the leading '>' and 1 for space at end
	uint16_t xpos = m_display.width() - (exitStrLen * fontWidth());

	if(opt == menus[menu_selected]->option_selected)
	{
		drawString(">", xpos, YPOS + (h * (opt + 1)));
	}
	strcpy_P (tmpStr, str);
	drawString(tmpStr, xpos + fontWidth(), YPOS + (h * (opt + 1)));
}
bool WatchMenu::updateMenu()
{
	m_display.setFont(m_font);

	bool bAnimating = false;

	if ( MENU_TYPE_STR == menus[menu_selected]->type)
	{
		menu_drawStr();
	}
	else
	{
		// Get the index to the sub menu
		int8_t optSel = menus[menu_selected]->option_selected;
		int8_t menuIndex = menus[menu_selected]->options[optSel]->menu_index;

		{
			// Display as regular icon
			// Get the first menu to start with...change later to get current
			bAnimating = menu_drawIcon();
		}
	}
	// Draw stuff
	if(menus[menu_selected]->drawFunc != NULL)
	{
		menus[menu_selected]->drawFunc();
	}
  return bAnimating;
}

bool WatchMenu::menu_drawIcon()
{
  const int16_t displayWidth = m_display.width();
  const int16_t displayHeight = m_display.height();

  bool bAnimating = true;

//	int x = 64;
  int x = displayWidth / 2;
  x -= 48 * menus[menu_selected]->option_selected;

  int16_t *animX = &menus[menu_selected]->animX;
  {
    int8_t speed;
    if (x > *animX)
    {
      speed = ((x - *animX) / 4) + 1;
      if (speed > 16)
		speed = 16;
      *animX += speed;
      if (x <= *animX)
		*animX = x;
    }
    else if (x < *animX)
    {
      speed = ((*animX - x) / 4) + 1;
      if (speed > 16)
      	speed = 16;
      *animX -= speed;
      if (x >= *animX)
		*animX = x;
    }
    else
    {
      bAnimating = false;
    }
  }

  x = *animX - 16;

  // Copy the data from PROGMEM...max 20 chars
  char tmpStr[20] = { 0 };
  strcpy_P (tmpStr, menus[menu_selected]->name);
  int16_t tempX;
  int16_t tempY;
  uint16_t w;
  uint16_t h;

  // Get the string height specifically.
  m_display.getTextBounds(tmpStr, 0, 0, &tempX, &tempY, &w, &h);
  drawCentreString(tmpStr, displayWidth / 2, YPOS + h, textSize);

  // Create image struct
  // FIX: struct uses heap, should use stack
  uint8_t fix = selectbar_topWidthPixels;
  s_image img = newImage((displayWidth / 2) - (selectbar_topWidthPixels / 2),
			 YPOS + 14, selectbar_top, fix, 8, m_inverted ? WHITE : BLACK, NOINVERT);

  // Draw ...
  ultraFastDrawBitmap (&img);

  // Draw ...
  img.y = YPOS + 42;
  img.bitmap = selectbar_bottom;
  ultraFastDrawBitmap(&img);

  img.y = YPOS + 16;
  img.width = 32;
  img.height = 32;

  // Display each menu option
  for (byte i = 0; i < menus[menu_selected]->num_options; i++)
  {
    if (x < displayWidth && x > -32)
    {
      img.x = x;
      img.bitmap =
	  menus[menu_selected]->options[i]->icon != NULL ?
	      menus[menu_selected]->options[i]->icon : menu_default;
      ultraFastDrawBitmap(&img);
    }
    x += 48;
  }

  uint8_t sel_opt = menus[menu_selected]->option_selected;
  strcpy_P (tmpStr, menus[menu_selected]->options[sel_opt]->name);
  // Get the string height specifically.
  m_display.getTextBounds(tmpStr, 0, 0, &tempX, &tempY, &w, &h);
  drawCentreString(tmpStr, displayWidth / 2, YPOS + 64 - (h / 2), textSize);

  return bAnimating;
}

void WatchMenu::ultraFastDrawBitmap (s_image* image)
{
  m_display.drawBitmap(image->x, image->y, image->bitmap, image->width, image->height, image->foreColour);
}

void WatchMenu::resetMenu ()
{
  // reset all the options back to default 0
  menu_selected = 0;

  for (int menuLoop = 0; menuLoop < num_menus; menuLoop++)
  {
    menus[menuLoop]->option_selected = 0;
    // Reset animation
	menus[menuLoop]->animX = m_display.width() /  2;
  }
}

void WatchMenu::setTextSize (uint8_t size)
{
	m_display.setTextSize(size);
	textSize = size;
}

/***************************************************************************************
** Function name:           drawCentreString
** Descriptions:            draw string across centre
***************************************************************************************/
void WatchMenu::drawCentreString(char *str, int dX, int poY, int size)
{
	int16_t tempX;
	int16_t tempY;
	uint16_t w;
	uint16_t h;
	// Get the string width
	m_display.getTextBounds(str, 0, 0, &tempX, &tempY, &w, &h);

	int poX = dX - w / 2;
	m_display.setCursor(poX, poY);
	m_display.setTextColor(m_inverted ? WHITE: BLACK, m_inverted ? BLACK : WHITE);
	m_display.print(str);
}

void WatchMenu::drawString(char* str, byte x, byte y)
{
	m_display.setTextColor(m_inverted ? WHITE : BLACK, m_inverted ? BLACK : WHITE);
	m_display.setCursor(x, y);
	m_display.print(str);
}

void WatchMenu::setFont(const GFXfont *font)
{
	m_display.setFont(font);
	m_font = (GFXfont *)font; // Save the font
	// Get font dimensions, build a ABC and get width  / 3
	int16_t tempX;
	int16_t tempY;
	uint16_t w;
	uint16_t h;
	// Get font dimensions
	GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(&m_font->glyph))[0]); // Get first char and assume all are same width
	m_fontWidth = pgm_read_byte(&glyph->xAdvance); // Width of definition is 16 bits wide but in pixels is only 10.
	// Get the string width
	m_display.getTextBounds(PSTR("A"), 0, 0, &tempX, &tempY, &w, &h);
	m_fontHeight = h;
}

GFXfont *WatchMenu::getFont(void)
{
	return m_font;
}

void WatchMenu::selectedOption(int8_t menu_index, int8_t option_index)
{
	menus[menu_index]->option_selected = option_index;
}

void WatchMenu::invertDisplay(bool state)
{
	m_inverted = state;
}
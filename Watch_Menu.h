/*********************************************************************
This is a library for Watch

Written by Mark Winney.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/
#ifndef _WATCH_MENU_H
#define _WATCH_MENU_H

#if ARDUINO >= 100
 #include "Arduino.h"
 #define WIRE_WRITE Wire.write
#else
 #include "WProgram.h"
  #define WIRE_WRITE Wire.send
#endif

#if defined(__SAM3X8E__) || defined(ARDUINO_ARCH_SAMD)
 typedef volatile RwReg PortReg;
 typedef uint32_t PortMask;
#endif
#ifdef __AVR__
  typedef volatile uint8_t PortReg;
  typedef uint8_t PortMask;
#endif

#define MENU_TYPE_STR	0
#define MENU_TYPE_ICON	1

#define MENU_EXIT              -99
#define OPTION_NEXT_PAGE       -1
#define OPTION_PREV_PAGE       -2
#define OPTION_EXIT_PAGE       -3

#define BLACK 0
#define WHITE 1
#define INVERSE 2

#define newImage(x, y, bitmap, width, height, foreColour, invert) \
(s_image){ \
	x, \
	y, \
	bitmap, \
	width, \
	height, \
	foreColour, \
	invert \
}

typedef struct
{
	int16_t x;
	int16_t y;
	const uint8_t* bitmap;
	uint8_t width;
	uint8_t height;
	uint8_t foreColour;
	bool invert;
}s_image;


typedef void (*pFunc)(void);

typedef struct
{
	int8_t num_items;
	char name[20];
	const uint8_t *icon;
	int8_t menu_index;
	int8_t invert_start;
	int8_t invert_length;
	pFunc func;
}s_option;

typedef struct
{
	int8_t num_options;
	char name[20];
	s_option **options; // Array of pointer to options
	int8_t option_selected;
	int8_t prev_menu;
	int8_t type;
	int16_t animX;  // menu animation X pos
	pFunc downFunc;
	pFunc upFunc;
	pFunc drawFunc;
}s_menu;

class WatchMenu
{
public:
	WatchMenu(Adafruit_SharpMem& display);
	void initMenu(uint8_t num);
	void createMenu(int8_t index, int8_t num_options, const char *name, int8_t menu_type = MENU_TYPE_ICON);
	void createMenu(int8_t index, int8_t num_options, const char *name, int8_t menu_type, pFunc downFunc, pFunc upFunc);
	void createOption(int8_t menu_index, int8_t opt_index, const char *name, const uint8_t *icon, pFunc actionFunc);
	void createOption(int8_t menu_index, int8_t opt_index, const char *name, const uint8_t *icon, uint8_t prev_menu_index);
	void createOption(int8_t menu_index, int8_t opt_index, pFunc actionFunc, uint8_t prev_menu_index);
	void createOption(int8_t menu_index, int8_t opt_index, const char *name, uint8_t prev_menu_index);
	void createOption(int8_t menu_index, int8_t opt_index, int16_t invert_start, int16_t invert_length, const char *name, const uint8_t *icon, pFunc actionFunc);

	bool updateMenu();
	void upOption(void);
	void downOption(void);
	bool menuDown(void);
	bool menuUp(void);
	bool selectOption(void);
	void resetMenu(void);
	bool menu_drawIcon();
	void setTextSize(uint8_t size);
	void drawString(char* str, byte x, byte y);
	void drawCentreString(char *str, int dX, int poY, int size);
	void setDownFunc(pFunc func);
	void setUpFunc(pFunc func);
	void setDrawFunc(pFunc func);
	void setFont(const GFXfont *font);
	GFXfont *getFont(void);
	void selectedOption(int8_t menu_index, int8_t option_index);
	uint8_t fontWidth(){ return m_fontWidth; };
	uint8_t fontHeight(){ return m_fontHeight; };
	void invertDisplay(bool state);


  private:
	void ultraFastDrawBitmap(s_image* image);
	void menu_drawStr();


	int8_t num_menus;
	s_menu **menus; //Array of pointers to menus
	uint8_t menu_selected;
	Adafruit_SharpMem& m_display;
	uint8_t textSize;
	GFXfont *m_font;
	uint8_t m_fontWidth;
	uint8_t m_fontHeight;
	bool m_inverted;
};


#endif

#include "Keyboard.h"

#define rowCount 7
#define colCount 5

char keyboard[colCount][rowCount] = {
  {'q', 'w', NULL, 'a',  KEY_LEFT_ALT, ' ', KEY_LEFT_CTRL}, // sym, ALT, mic
  {'e', 's', 'd',  'p',  'x',  'z', KEY_LEFT_SHIFT}, // left shift
  {'r', 'g', 't',  KEY_RIGHT_SHIFT, 'v',  'c', 'f'}, // right shift
  {'u', 'h', 'y',  KEY_RETURN, 'b',  'n', 'j'}, // Enter
  {'o', 'l', 'i', KEY_BACKSPACE, '$', 'm', 'k'} // Backspace
};
char keyboard_symbol[colCount][rowCount] = {
  {'#', '1', NULL, '*',  KEY_TAB, ' ', '0'}, // sym, ALT, SPACE
  {'2', '4', '5',  '@',  '8',  '7', KEY_LEFT_SHIFT}, // left shift
  {'3', '/', '(',  KEY_RIGHT_SHIFT, '?',  '9', '6'}, // right shift
  {'_', ':', ')',  NULL, '!',  ',', ';'}, // Enter
  {'+', '"', '-', KEY_BACKSPACE, NULL, '.', '\''} // $/Vol
};
char keyboard_cursor[colCount][rowCount] = {
  {KEY_ESC, KEY_UP_ARROW, NULL, KEY_LEFT_ARROW,  KEY_TAB, ' ', '0'}, // sym, ALT, SPACE
  {'2', KEY_DOWN_ARROW, KEY_RIGHT_ARROW,  KEY_F15,  '8',  '7', KEY_LEFT_SHIFT}, // left shift
  {'3', '/', '(',  KEY_RIGHT_SHIFT, '?',  '9', '6'}, // right shift
  {KEY_F7, ':', ')',  KEY_F4, '!', ',', ';'}, // Enter
  {KEY_F6, '"', KEY_F8, KEY_F3, KEY_F5, '.', '\''} // $/Vol
};
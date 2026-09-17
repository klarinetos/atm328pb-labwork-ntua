/* PDF pp. 12-13: name starting at column 3 (1-based). */
#include "exam_helpers.h"
int main(void) {
    exam_lcd_init();
    lcd_command(0x82); /* Two positions from column 1, not three. */
    lcd_string((const unsigned char *)"STAVROULA"); /* Change the name here. */
    while (1) {}
}

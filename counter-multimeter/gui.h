#ifndef _GUI_H_
#define _GUI_H_

#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "includes/lcd.h"
#include "joy.h"
#include "systime.h"
#include "counter.h"

#define GUI_MEASURE_VOLTAGE_DC			0
#define GUI_MEASURE_VOLTAGE_AC			1
#define GUI_MEASURE_CURRENT_DC			2
#define GUI_MEASURE_CURRENT_AC			3
#define GUI_MEASURE_RESISTANCE			4
#define GUI_MEASURE_CONTINUITY			5
#define GUI_MEASURE_FREQUENCY			6
#define GUI_MEASURE_DUTY				7

#define GUI_MAX_MENU_NAME_LENGTH		5
#define GUI_NUM_MENU_ENTRIES			10
#define GUI_NUM_MEASUREMENT_ENTRIES		8

#define GUI_MAX_RANGES_PER_ENTRY		7
#define GUI_MAX_RANGE_NAME_LENGTH		11

struct {
	uint8_t selectedEntry;
	uint8_t selectedRanges[GUI_NUM_MENU_ENTRIES];
	uint8_t measurementActive;
	int32_t measurementResult;
} gui;

void gui_MainMenu(void);
void gui_DisplayMainMenu(void);
void gui_HandleUserInput(void);

void gui_TakeMeasurement(void);

/**
 * \brief Creates a string from an unsigned integer
 *
 * The string be will be created with leading zeros
 * and an optional fixed decimal point
 * \param value     Integer value which will be written to string
 * \param dest      Pointer to (big enough!) char array
 * \param digits    Number of displayed digits (not counting the decimal point).
 *                  If value has more digits than specified only the last
 *                  digits will be displayed. If value has less digits the
 *                  string will be filled with leading zeros
 * \param dot       Number of digits behind decimal point. If 0 the decimal point
 *                  will be omitted
 */
void gui_string_fromInt(int32_t value, char *dest, uint8_t digits, uint8_t dot);

#endif

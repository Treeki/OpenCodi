#include "opencodi.h"

static char console[2048];
static lv_obj_t *consoleObj = NULL;
static lv_obj_t *timeObj = NULL;
static lv_obj_t *batteryLabelObj = NULL;

void consolePush(const char *str) {
	if (consoleObj == NULL)
		return;
	int toAdd = strlen(str);
	int currentLen = strlen(console);
	int maxLen = sizeof(console) - 1;
	if ((toAdd + currentLen) >= maxLen) {
		int toRemove = maxLen - toAdd;
		memmove(console, &console[toRemove], currentLen - toRemove);
		currentLen -= toRemove;
	}
	strcpy(&console[currentLen], str);
	lv_label_set_static_text(consoleObj, console);
}


void updateTimeLabel() {
	char buf[32];
	uint64_t timestamp = ocTime();
	// TODO make this more elegant
	int work = timestamp % 86400;
	int hours = work / 3600;
	work -= (hours * 3600);
	int minutes = work / 60;
	work -= (minutes * 60);
	int seconds = work;
	sprintf(buf, "%02d:%02d:%02d", hours, minutes, seconds);
	lv_label_set_text(timeObj, buf);
}


void brightnessSliderCB(lv_obj_t *slider, lv_event_t event) {
	if (event == LV_EVENT_VALUE_CHANGED) {
		ocDisplaySetBrightness(lv_slider_get_value(slider));
	}
}

void ocUiSetup() {
	lv_obj_t *tabs = lv_tabview_create(lv_scr_act(), NULL);
	lv_obj_t *tab;

	// Main Tab
	tab = lv_tabview_add_tab(tabs, "Home");

	lv_obj_t *slider = lv_slider_create(tab, NULL);
	lv_slider_set_range(slider, 10, 160); // TODO: check me?
	lv_slider_set_value(slider, 40, LV_ANIM_OFF);
	lv_obj_set_pos(slider, 10, 10);
	lv_obj_set_width(slider, SCREEN_WIDTH - 20);
	lv_obj_set_event_cb(slider, brightnessSliderCB);

	timeObj = lv_label_create(tab, NULL);
	lv_obj_align(timeObj, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	batteryLabelObj = lv_label_create(tab, NULL);
	lv_obj_align(batteryLabelObj, timeObj, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

	// Console
	tab = lv_tabview_add_tab(tabs, "Console");
	consoleObj = lv_label_create(tab, NULL);
	lv_label_set_long_mode(consoleObj, LV_LABEL_LONG_BREAK);
	lv_obj_set_width(consoleObj, lv_page_get_fit_width(tab));
	console[0] = 0;
	lv_label_set_static_text(consoleObj, console);
}


const char *selectIcon(int percentage) {
	if (percentage > 85) return LV_SYMBOL_BATTERY_FULL;
	if (percentage > 60) return LV_SYMBOL_BATTERY_3;
	if (percentage > 35) return LV_SYMBOL_BATTERY_2;
	if (percentage > 10) return LV_SYMBOL_BATTERY_1;
	return LV_SYMBOL_BATTERY_EMPTY;
}

void ocUiSetBatteryLevel(bool charging, int percentage) {
	char buf[32];
	sprintf(buf, charging ? "%s Charging - %d%%" : "%s %d%%", selectIcon(percentage), percentage);
	lv_label_set_text(batteryLabelObj, buf);
}


void ocUiUpdatePerSecond() {
	updateTimeLabel();
}




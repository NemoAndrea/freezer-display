#include "utils.h"

extern const lv_img_dsc_t splash_screen_logo;
extern const lv_img_dsc_t splash_screen_footer;

void build_splash_screen() {

    // logo
    lv_obj_t * img_obj = lv_img_create(lv_scr_act());
    lv_img_set_src(img_obj, &splash_screen_logo);
    lv_obj_align(img_obj,LV_ALIGN_CENTER, 0, -240); 
    
    // open hardware
    lv_obj_t * img_obj_f = lv_img_create(lv_scr_act());
    lv_img_set_src(img_obj_f, &splash_screen_footer);
    lv_obj_align(img_obj_f,LV_ALIGN_CENTER, 0, 750); 

    // Automatic Reboot Notice
    lv_obj_t* label_support = lv_label_create(lv_scr_act());
    lv_obj_align(label_support, LV_ALIGN_CENTER, 0, 850);
    lv_obj_set_style_text_font(label_support, &lv_font_montserrat_20, 0);
    lv_label_set_text_fmt(label_support, "For support visit %s", freezer_config.SUPPORT_PAGE_URL.c_str());
    lv_obj_set_style_text_align(label_support, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(label_support, get_grayscale_color(6), 0);
}
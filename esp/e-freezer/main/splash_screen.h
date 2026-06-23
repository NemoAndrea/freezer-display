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
}
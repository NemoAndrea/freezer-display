extern const lv_img_dsc_t error_no_api;
extern const lv_img_dsc_t error_no_internet;
extern const lv_img_dsc_t error_no_wifi;

enum VisualError { no_wifi, no_internet, no_api };

void build_error_screen(VisualError vis_error,lv_obj_t* screen) {

    lv_obj_clean(screen);

    // error type image/icon
    lv_obj_t * img_obj = lv_img_create(screen);
    lv_obj_align(img_obj,LV_ALIGN_TOP_LEFT, 780, 160); 
    
    // main heading

    lv_obj_t* label_sys_error = lv_label_create(screen);
    lv_obj_align(label_sys_error, LV_ALIGN_TOP_LEFT, 132, 570);
    lv_label_set_text(label_sys_error, "System Issue");
    lv_obj_set_style_text_font(label_sys_error, &lv_font_montserrat_32, 0);

    lv_obj_t* label_error_identifier = lv_label_create(screen);
    lv_obj_align(label_error_identifier, LV_ALIGN_TOP_LEFT, 132, 652);
    lv_obj_set_style_text_font(label_error_identifier, &lv_font_montserrat_48, 0);

    lv_obj_t* label_error_explainer = lv_label_create(screen);
    lv_obj_align(label_error_explainer, LV_ALIGN_TOP_LEFT, 132, 750);
    lv_obj_set_style_text_font(label_error_explainer, &lv_font_montserrat_32, 0);
    lv_obj_set_size(label_error_explainer, lv_pct(80), LV_SIZE_CONTENT); 
    lv_label_set_long_mode(label_error_explainer, LV_LABEL_LONG_WRAP);

    // Automatic Reboot Notice
    lv_obj_t* label_reboot = lv_label_create(screen);
    lv_obj_align(label_reboot, LV_ALIGN_CENTER, 0, 800);
    lv_obj_set_style_text_font(label_reboot, &lv_font_montserrat_20, 0);
    lv_label_set_text_fmt(label_reboot, "Device will automatically retry connection.\n If the issue persists, consult %s", SUPPORT_PAGE_URL);
    lv_obj_set_style_text_align(label_reboot, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    // Solutions

    lv_obj_t* label_poss_causes = lv_label_create(screen);
    lv_obj_align(label_poss_causes, LV_ALIGN_TOP_LEFT, 139, 1087);
    lv_label_set_text(label_poss_causes, "Possible Causes");
    lv_obj_set_style_text_font(label_poss_causes, &lv_font_montserrat_20, 0);


    lv_obj_t* causes_flex = lv_obj_create(screen);
    lv_obj_align(causes_flex, LV_ALIGN_TOP_LEFT, 139, 1142);
    lv_obj_set_size(causes_flex, lv_pct(60), LV_SIZE_CONTENT); 
    lv_obj_set_style_text_font(causes_flex, &lv_font_montserrat_32, 0);
    lv_obj_set_flex_flow(causes_flex, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(causes_flex, 40, LV_PART_MAIN);
    lv_obj_set_style_border_width(causes_flex, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(causes_flex, 0, LV_PART_MAIN);

    switch (vis_error) {
        case VisualError::no_wifi: {
            lv_img_set_src(img_obj, &error_no_wifi);
            lv_label_set_text(label_error_identifier, "No Wi-Fi Connection");
            lv_label_set_text(label_error_explainer, "The freezer-display is unable to connect to a Wi-Fi access point.");

            lv_obj_t* cause_1 = lv_label_create(causes_flex);
            lv_label_set_text(cause_1, "Wi-Fi nodes are currently unavailable");
            lv_obj_t* cause_2 = lv_label_create(causes_flex);
            lv_label_set_text(cause_2, "The display is out of range of access points");
            lv_obj_t* cause_3 = lv_label_create(causes_flex);
            lv_label_set_text(cause_3, "The display Wi-Fi credentials are no longer valid");
            lv_obj_t* cause_4 = lv_label_create(causes_flex);
            lv_label_set_text(cause_4, "The Wi-Fi connection is actively blocked by changed network policies");
            break;
        } 

        case VisualError::no_internet: {
            lv_img_set_src(img_obj, &error_no_internet);
            lv_label_set_text(label_error_identifier, "No Internet Access");
            lv_label_set_text(label_error_explainer, "The freezer-display was able to connect to Wi-Fi, but is not able to access the internet through this connection.");
        
            lv_obj_t* cause_1 = lv_label_create(causes_flex);
            lv_label_set_text(cause_1, "The faciltiy network has no access to the internet");
            lv_obj_t* cause_2 = lv_label_create(causes_flex);
            lv_label_set_text(cause_2, "The endpoint that checks for internet access is down/blocked the device");
            lv_obj_t* cause_3 = lv_label_create(causes_flex);
            lv_label_set_text(cause_3, "The display Wi-Fi credentials are no longer valid");
            break;
        }

        case VisualError::no_api: {
            lv_img_set_src(img_obj, &error_no_api);
            lv_label_set_text(label_error_identifier, "No API Connection");
            lv_label_set_text(label_error_explainer, "The freezer-display has an internet connection, but cannot fetch freezer data from the eLabJournal API");

            lv_obj_t* cause_1 = lv_label_create(causes_flex);
            lv_label_set_text(cause_1, "The eLabJournal API token is no longer valid");
            lv_obj_t* cause_2 = lv_label_create(causes_flex);
            lv_label_set_text(cause_2, "The API service is down");
            lv_obj_t* cause_3 = lv_label_create(causes_flex);
            lv_label_set_text(cause_3, "The API endpoint is being blocked on the network that you are using");
            break;
        }

    }

    // Force all child labels to wrap to the parent's width
    for (uint32_t i = 0; i < lv_obj_get_child_cnt(causes_flex); i++) {
        lv_obj_t * child = lv_obj_get_child(causes_flex, i);
        
        // 1. Set width to 100% of the parent flex container
        lv_obj_set_width(child, lv_pct(100));
        
        // 2. Enable automatic multi-line wrapping
        lv_label_set_long_mode(child, LV_LABEL_LONG_WRAP);
    }

    lv_scr_load(screen);
}


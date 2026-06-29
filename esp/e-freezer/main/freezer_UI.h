#include "freezer_api.h"
#include "utils.h"

#include <chrono>
#include <format>
#include <string>
#include <ctime>

struct GridDsc {
    std::vector<lv_coord_t> cols;
    std::vector<lv_coord_t> rows;
};

static void grid_dsc_delete_cb(lv_event_t* e) {
    delete static_cast<GridDsc*>(lv_event_get_user_data(e));
}


lv_obj_t* create_fixed_grid(lv_obj_t* parent, uint8_t n_cols, uint8_t n_rows) {
    auto* dsc = new GridDsc{
        std::vector<lv_coord_t>(n_cols + 1, LV_GRID_FR(1)),
        std::vector<lv_coord_t>(n_rows + 1, LV_GRID_FR(1))
    };
    dsc->cols[n_cols] = LV_GRID_TEMPLATE_LAST;
    dsc->rows[n_rows] = LV_GRID_TEMPLATE_LAST;

    lv_obj_t* grid = lv_obj_create(parent);
    lv_obj_set_grid_dsc_array(grid, dsc->cols.data(), dsc->rows.data());
    lv_obj_add_event_cb(grid, grid_dsc_delete_cb, LV_EVENT_DELETE, dsc);  // lifetime hook stuff
    lv_obj_set_style_pad_row(grid, 24, 0);  // grid-gap vertical in px
    lv_obj_set_style_pad_column(grid, 24, 0);  // grid-gap horizontal in px

    lv_obj_set_size(grid, 1360, 866);

    lv_obj_set_style_radius(grid, 0, 0);
    lv_obj_set_style_bg_color(grid, get_grayscale_color(14), 0);
    lv_obj_set_style_border_width(grid, 0, 0);  // no border

    return grid;
}


void build_drawer(std::tuple<FreezerAPI::FreezerBox, FreezerAPI::FreezerBox> drawer, lv_obj_t* grid_top, lv_obj_t* grid_bottom, lv_obj_t* screen) {
        auto [box_1, box_2] = drawer; 
        lv_obj_t* box_item;
        if (box_1.compartment_idx == 0)  // check if the box set is in the first compartment (box boxes are trivially in the same comp)
        {
            box_item = lv_obj_create(grid_top);
        } else {
            box_item = lv_obj_create(grid_bottom);
        }
        lv_obj_set_grid_cell(box_item, LV_GRID_ALIGN_STRETCH, box_1.column_idx, 1,
                LV_GRID_ALIGN_STRETCH, box_1.row_idx, 1);
        lv_obj_set_style_radius(box_item, 0, 0);

        // misc
        lv_obj_set_scrollbar_mode(box_item, LV_SCROLLBAR_MODE_OFF);  // prevent scrollbars
        lv_obj_set_style_pad_all(box_item, 0, 0); // remove container padding

        // drawer divider vertical line
        lv_obj_t * vertical_bar = lv_obj_create(box_item);
        lv_obj_set_width(vertical_bar, 3);  // 3 pixels, matching the box border
        lv_obj_set_height(vertical_bar, lv_pct(100));
        lv_obj_align(vertical_bar, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(vertical_bar, get_grayscale_color(0), 0);
        lv_obj_set_style_bg_opa(vertical_bar, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(vertical_bar, 0, 0);
        lv_obj_set_style_border_width(vertical_bar, 0, 0);  // override this as it would draw over bg

        // text (owners)
        lv_obj_t* label_box_1 = lv_label_create(box_item);
        lv_label_set_text(label_box_1, (box_1.owner).c_str());
        lv_obj_align(label_box_1, LV_ALIGN_TOP_LEFT, 0, -4);
        lv_obj_set_style_pad_hor(label_box_1, 3, 0);
        
        lv_obj_set_style_bg_color(label_box_1, get_grayscale_color(0), 0); // black background
        lv_obj_set_style_bg_opa(label_box_1, LV_OPA_COVER, 0);

        lv_obj_set_style_text_color(label_box_1, get_grayscale_color(15), 0);
        lv_obj_set_style_text_font(label_box_1, &lv_font_montserrat_20, 0);

        // if box 2 has the same owner as box1, then we can skip the owner label
        if (box_1.owner != box_2.owner) {
            lv_obj_t* label_box_2 = lv_label_create(box_item);
            lv_label_set_text(label_box_2, (box_2.owner).c_str());
            lv_obj_align(label_box_2, LV_ALIGN_TOP_MID, 0, -4);
            lv_obj_set_style_translate_x(label_box_2, lv_pct(50), 0);
            lv_obj_set_style_pad_hor(label_box_2, 4, 0);

            lv_obj_set_style_bg_color(label_box_2, get_grayscale_color(0), 0); // black background
            lv_obj_set_style_bg_opa(label_box_2, LV_OPA_COVER, 0);

            lv_obj_set_style_text_color(label_box_2, get_grayscale_color(15), 0);
            lv_obj_set_style_text_font(label_box_2, &lv_font_montserrat_20, 0);
        }

        // text (samples)
        lv_obj_t* list_container_1 = lv_obj_create(box_item);
        lv_obj_set_size(list_container_1, lv_pct(48), LV_SIZE_CONTENT); // Width capped at 45% of parent
        lv_obj_set_flex_flow(list_container_1, LV_FLEX_FLOW_COLUMN);     // Stack items vertically
        lv_obj_set_style_pad_all(list_container_1, 0, 0);               // Strip padding
        lv_obj_set_style_bg_opa(list_container_1, LV_OPA_TRANSP, 0); // Hide background container
        lv_obj_set_style_border_width(list_container_1, 0, 0);
        lv_obj_align(list_container_1, LV_ALIGN_TOP_LEFT, 4, 18);
        for (const auto& sample : box_1.samples) {
            lv_obj_t* line = lv_label_create(list_container_1);
            lv_label_set_long_mode(line, LV_LABEL_LONG_DOT);
            lv_obj_set_width(line, lv_pct(100)); 
            lv_obj_set_style_max_height(line, 26, 0); // Roughly the height of a 22px font line
            lv_label_set_text(line, sample.c_str());
            lv_obj_set_style_text_font(line, &lv_font_montserrat_22, 0);
            lv_obj_set_style_pad_all(list_container_1, 0, 0);
            lv_obj_set_style_pad_row(list_container_1, 0, 0);
        }

        lv_obj_t* list_container_2 = lv_obj_create(box_item);
        lv_obj_set_size(list_container_2, lv_pct(48), LV_SIZE_CONTENT); // Width capped at 45% of parent
        lv_obj_set_flex_flow(list_container_2, LV_FLEX_FLOW_COLUMN);     // Stack items vertically
        lv_obj_set_style_pad_all(list_container_2, 0, 0);               // Strip padding
        lv_obj_set_style_bg_opa(list_container_2, LV_OPA_TRANSP, 0); // Hide background container
        lv_obj_set_style_border_width(list_container_2, 0, 0);
        if (box_1.owner != box_2.owner) {lv_obj_align(list_container_2, LV_ALIGN_TOP_MID, 4, 18);}
        else {lv_obj_align(list_container_2, LV_ALIGN_TOP_MID, 6, 2);}\
        lv_obj_set_style_translate_x(list_container_2, lv_pct(50), 0);
        for (const auto& sample : box_2.samples) {
            lv_obj_t* line = lv_label_create(list_container_2);
            lv_label_set_long_mode(line, LV_LABEL_LONG_DOT);
            lv_obj_set_width(line, lv_pct(100)); 
            lv_obj_set_style_max_height(line, 26, 0); // Roughly the height of a 22px font line
            lv_label_set_text(line, sample.c_str());
            lv_obj_set_style_text_font(line, &lv_font_montserrat_22, 0);
            lv_obj_set_style_pad_all(list_container_2, 0, 0);
            lv_obj_set_style_pad_row(list_container_2, 0, 0);
        }
        
        // border
        lv_obj_set_style_border_width(box_item, 3, 0);
        lv_obj_set_style_border_color(box_item, get_grayscale_color(0), 0);

        // drop shadow
        lv_obj_set_style_shadow_width(box_item, 1, 0);  // no blur
        lv_obj_set_style_shadow_spread(box_item, 1, 0);
        lv_obj_set_style_shadow_opa(box_item, LV_OPA_COVER, 0); // no opacity
        lv_obj_set_style_shadow_ofs_x(box_item, 9, 0);
        lv_obj_set_style_shadow_ofs_y(box_item, 9, 0);
        lv_obj_set_style_shadow_color(box_item, get_grayscale_color(0), 0);
}


void build_header(FreezerAPI::FreezerContent freezer, lv_obj_t* screen) {
    lv_obj_t* header = lv_obj_create(screen);
    lv_obj_set_style_pad_all(header, 0, 0);
    static lv_coord_t column_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3), LV_GRID_FR(3), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};  
    static lv_coord_t row_dsc[] = { LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};  // take up all vertical space
    lv_obj_set_grid_dsc_array(header, column_dsc, row_dsc);
    lv_obj_set_size(header, 1200, 100);  // matching width (manually determined) is 1320px
    lv_obj_set_style_border_width(header, 0, 0);  // no border for the grid itself
    lv_obj_set_style_pad_row(header, 0, 0);  // no gap
    lv_obj_set_style_pad_column(header, 0, 0);  // no gap
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, -16);
    // set black border for child elements (better than setting on each child)
    lv_obj_set_style_bg_color(header, get_grayscale_color(0), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_column(header, 3, 0);
    lv_obj_set_style_border_width(header, 3, 0);
    lv_obj_set_style_border_color(header, get_grayscale_color(0), 0);

    int label_padding_L = 16;  // px
    int label_padding_B = -12;  // px

    // logo box 
    lv_obj_t* logobox = lv_obj_create(header);
    lv_obj_set_style_pad_all(logobox, 0, 0);
    lv_obj_t* label_logo = lv_label_create(logobox);
    lv_obj_align(label_logo, LV_ALIGN_BOTTOM_LEFT, label_padding_L, label_padding_B);
    lv_label_set_text_fmt(label_logo, "%s\n%s", "E-freezer", "Open Harware"); // name in system + the storage ID in config
    lv_obj_set_style_text_font(label_logo, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_logo, get_grayscale_color(6), 0);
    lv_obj_set_scrollbar_mode(logobox, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(logobox, 0, 0);
    lv_obj_set_style_border_width(logobox, 0, 0);
    lv_obj_set_grid_cell(logobox, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    // show name and storagelayerID of freezer
    lv_obj_t* namebox = lv_obj_create(header);
    lv_obj_set_style_pad_all(namebox, 0, 0);
    lv_obj_t* label = lv_label_create(namebox);
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, label_padding_L, label_padding_B);
    lv_label_set_text_fmt(label, "%s\n%s%d", freezer.label.c_str(), "Layer ID: ", STORAGE_LAYER_ID); // name in system + the storage ID in config
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, get_grayscale_color(6), 0);
    lv_obj_set_scrollbar_mode(namebox, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(namebox, 0, 0);
    lv_obj_set_style_border_width(namebox, 0, 0);
    lv_obj_set_grid_cell(namebox, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    
    
    // show API endpoint and git commit hash
    lv_obj_t* metabox = lv_obj_create(header);
    lv_obj_set_style_pad_all(metabox, 0, 0);
    lv_obj_t* label_meta = lv_label_create(metabox);
    lv_obj_align(label_meta, LV_ALIGN_BOTTOM_LEFT, label_padding_L, label_padding_B);
    lv_label_set_text_fmt(label_meta, "%s\n%s%s", API_ENDPOINT + 8, "Git commit: ", GIT_COMMIT_HASH);
    lv_obj_set_style_text_font(label_meta, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label_meta, get_grayscale_color(6), 0);
    lv_obj_set_scrollbar_mode(metabox, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(metabox, 0, 0);
    lv_obj_set_style_border_width(metabox, 0, 0);
    lv_obj_set_grid_cell(metabox, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);


    // show information about the last update time 
    lv_obj_t* retrievedbox = lv_obj_create(header);
    lv_obj_set_style_pad_all(retrievedbox, 0, 0);
    lv_obj_t* label_retrieved = lv_label_create(retrievedbox);
    lv_obj_align(label_retrieved, LV_ALIGN_BOTTOM_LEFT, label_padding_L, label_padding_B);
    lv_obj_set_style_text_color(label_retrieved, get_grayscale_color(0), 0);  // higher contrast
    // we need to convert our chrono timestamp to something that understands timezones
    std::time_t raw_time = std::chrono::system_clock::to_time_t(freezer.time_retrieved);
    std::tm local_tm = *std::localtime(&raw_time);
    char time_buffer[64];
    std::strftime(time_buffer, sizeof(time_buffer), "%B %d %H:%M", &local_tm);
    lv_label_set_text_fmt(label_retrieved, "Last Updated\n%s", time_buffer);
    lv_obj_set_style_text_font(label_retrieved, &lv_font_montserrat_22, 0);
    lv_obj_set_scrollbar_mode(retrievedbox, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_radius(retrievedbox, 0, 0);
    lv_obj_set_style_border_width(retrievedbox, 0, 0);
    lv_obj_set_grid_cell(retrievedbox, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
}


void build_freezer_UI(FreezerAPI::FreezerContent freezer, lv_obj_t* screen) {
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, get_grayscale_color(14), 0);
    
    auto [n_comp, n_cols, n_rows] = freezer.dimensions;

    ESP_LOGI("FreezerUI", "Drawing Freezer with ncomp %d, n_cols %d, n_rows %d", n_comp, n_cols, n_rows);

    // add header bar (also a grid)
    build_header(freezer, screen);

    // create grid
    lv_obj_t * grid_top = create_fixed_grid(screen, n_cols, n_rows);
    lv_obj_t * grid_bottom = create_fixed_grid(screen, n_cols, n_rows);
    
    lv_obj_align(grid_top, LV_ALIGN_CENTER, -2, -410); 
    lv_obj_align(grid_bottom, LV_ALIGN_CENTER, -2, 475); 

    // now add items to the grid
    for (auto drawer: freezer.drawers) {
        build_drawer(drawer, grid_top, grid_bottom, screen);
    }
}
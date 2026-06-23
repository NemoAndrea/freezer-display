#include "freezer_api.h"

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
    lv_obj_add_event_cb(grid, grid_dsc_delete_cb, LV_EVENT_DELETE, dsc);

    lv_obj_set_size(grid, 1300, 800);

    return grid;
}

void build_freezer_UI(FreezerAPI::FreezerContent freezer, lv_obj_t* screen) {
    lv_obj_clean(screen);  // reset the interface (TODO maybe reuse some aspects later)
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x202020), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    auto [n_comp, n_cols, n_rows] = freezer.dimensions;

    ESP_LOGI("FreezerUI", "\n Drawing the Freezer Content to Display...");
    ESP_LOGI("FreezerUI", "With ncomp %d, n_cols %d, n_rows %d", n_comp, n_cols, n_rows);

    // create grid
    lv_obj_t * grid_top = create_fixed_grid(screen, n_cols, n_rows);
    lv_obj_t * grid_bottom = create_fixed_grid(screen, n_cols, n_rows);

    lv_obj_align(grid_top, LV_ALIGN_CENTER, 0, -400); 
    lv_obj_align(grid_bottom, LV_ALIGN_CENTER, 0, 440); 

    // now add items to the grid
    for (FreezerAPI::FreezerBox box: freezer.boxes) {
        lv_obj_t* box_item;
        if (box.compartment_idx == 0)
        {
            box_item = lv_obj_create(grid_top);
        } else {
            box_item = lv_obj_create(grid_bottom);
        }
        lv_obj_set_grid_cell(box_item, LV_GRID_ALIGN_STRETCH, box.column_idx, 1,
                LV_GRID_ALIGN_STRETCH, box.row_idx, 1);
        lv_obj_t* label = lv_label_create(box_item);
        lv_label_set_text(label, box.label.c_str());
        lv_obj_center(label);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
    }
}
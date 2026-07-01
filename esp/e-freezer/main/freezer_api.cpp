#include <stdio.h>
#include <utility>
#include <vector>
#include <stdlib.h>
#include <algorithm>  // for std::sort
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h" // <-- Required for secure HTTPS bundle attaching
#include <string>
#include "cJSON.h"
#include <chrono>  // time functions

#include "freezer_api.h"

static const char* TAG = "FreezerAPI";

struct ResponseBuffer {
    char *data = nullptr;
    int len = 0;
};


// only for parsing JSON
struct LayerItem {
    int id;
    std::string name;
    int ctr;
};

// simple utility function for std::sort
static bool compare_layeritem_by_name(const LayerItem& a, const LayerItem& b) {
    return a.name < b.name;
};


// Event handler called by esp_http_client as data arrives
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    auto *resp = static_cast<ResponseBuffer *>(evt->user_data);

    if (evt->event_id == HTTP_EVENT_ON_DATA) {
        // Reallocate buffer to fit new chunk
        char *new_data = static_cast<char *>(
            realloc(resp->data, resp->len + evt->data_len + 1));
        if (!new_data) {
            ESP_LOGE(TAG, "Failed to allocate response buffer");
            return ESP_FAIL;
        }
        resp->data = new_data;
        memcpy(resp->data + resp->len, evt->data, evt->data_len);
        resp->len += evt->data_len;
        resp->data[resp->len] = '\0'; // keep it null-terminated
    }

    return ESP_OK;
}

std::string FreezerAPI::build_url(const char* sub_path) const {
    std::string endpointStr(_endpoint);
    
    // Check if endpoint is empty
    if (endpointStr.empty()) {
        return sub_path;
    }

    // Check for trailing slash
    if (endpointStr.back() == '/') {
        return endpointStr + sub_path;
    } else {
        return endpointStr + "/" + sub_path;
    }
}

std::pair<esp_http_client_handle_t, esp_err_t> FreezerAPI::get_API_response(esp_http_client_config_t config, int timeout_ms) {
    config.method = HTTP_METHOD_GET;
    config.timeout_ms = timeout_ms; // 5 second connection timeout

    // get the TLS verification stuff 
    config.crt_bundle_attach = esp_crt_bundle_attach;

    // initialise the HTTP client handle
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // set up request Headers
    esp_http_client_set_header(client, "Authorization", _token);
    esp_http_client_set_header(client, "accept", "application/json");

    // make the request
    esp_err_t err = esp_http_client_perform(client);

    return {client, err};
}


bool FreezerAPI::endpoint_available() {
    ESP_LOGI(TAG, "Fetching from API... ");

    // little bit of a pointless check but lets be safe
    if (_token == nullptr || _endpoint == nullptr) {
        ESP_LOGE(TAG, "ELAB_API_TOKEN environment variable is not set!");
        return false;
    }

    
    esp_http_client_config_t config= {};
    auto url = build_url("storage");
    config.url = url.c_str();  // form our API check URL (BASEURL + "storage")
    auto [client, err] = FreezerAPI::get_API_response(config, 5000);

    if (err == ESP_OK) {
        // Fetch the HTTP Status Code
        int status_code = esp_http_client_get_status_code(client);

        if (status_code != 200) {
            ESP_LOGW(TAG, "Did not get 200 response from API, got instead: %d", status_code);
            return false;
        } else {
            ESP_LOGI(TAG, "API is all good, key is valid!");
        }
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed completely: %s", esp_err_to_name(err));
        return false;
    }

    // 6. Clean up memory allocations used by the client connection
    esp_http_client_cleanup(client);
    return true;
}

cJSON* FreezerAPI::get_json(const char* url) {
    ResponseBuffer resp;

    esp_http_client_config_t config= {};
    config.user_data = &resp;
    config.url = url;
    config.event_handler = http_event_handler;

    auto [client, err] = FreezerAPI::get_API_response(config, 5000);

    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);

        if (status_code != 200) {
            ESP_LOGW(TAG, "Did not get 200 response from API, got instead: %d", status_code);
        } else {
            //ESP_LOGI(TAG, "HTTP status code 200 for freezer with ID %d", _freezer_id);
            
            // Read the data into your buffer
            if (resp.data) {
                cJSON *root = cJSON_Parse(resp.data);

                if (root) {
                    esp_http_client_cleanup(client);
                    return root;
                } else {
                    ESP_LOGE(TAG, "Failed to parse JSON response");
                }
            }  
        }
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed completely: %s", esp_err_to_name(err));
    }

    return NULL;
}





FreezerAPI::FreezerContent FreezerAPI::get_freezer_content() { 
    ESP_LOGI(TAG, "Getting content from Freezer with storageID %d... ", _freezer_id);


    // get freezer name
    auto url = build_url(("storageLayers/"+ std::to_string(_freezer_id)).c_str());
    cJSON* freezer_root = get_json(url.c_str());
    std::string freezer_name = cJSON_GetObjectItemCaseSensitive(freezer_root, "name")->valuestring;
    ESP_LOGI(TAG, "Name of Storage Unit: %s", freezer_name.c_str());
    cJSON_Delete(freezer_root);
    
    // start building up freezer content (compartments, columns, drawers, boxes)

    int idx_comp = 0;
    int idx_col = 0;
    int idx_row = 0;  // these are the drawers in eLabJournal
    // some rows/columns may be empty, but we still want to get the correct 'grid' dimensions of freezer
    int max_idx_col = 0;  
    int max_idx_row = 0;  

    std::vector<std::tuple<FreezerBox, FreezerBox>> box_vec;  // intialise
    FreezerContent content(freezer_name, box_vec,
         std::chrono::system_clock::now(),
        {idx_comp, idx_col, idx_row});
    
    
    // start by looping over the freezer to yield compartments 
    url = build_url(("storageLayers/"+ std::to_string(_freezer_id) + "/childLayers").c_str());
    cJSON* freezer_compartments = get_json(url.c_str());
    cJSON* compartment_arr = cJSON_GetObjectItemCaseSensitive(freezer_compartments, "data");
    cJSON* compartment = NULL;
    std::vector<LayerItem> compartments;
    cJSON_ArrayForEach(compartment, compartment_arr) {
        int compartment_id = cJSON_GetObjectItemCaseSensitive(compartment, "storageLayerID")->valueint;
        std::string compartment_name = cJSON_GetObjectItemCaseSensitive(compartment, "name")->valuestring;
        LayerItem item(compartment_id, compartment_name, idx_comp);
        compartments.push_back(item);
        idx_comp++;
    }
    cJSON_Delete(freezer_compartments);
    

    // loop over compartments to yield columns
    std::sort(compartments.begin(), compartments.end(), compare_layeritem_by_name); // sort alphabetically
    for (const LayerItem& compartment: compartments) {
        ESP_LOGI(TAG, "Compartment: %s", compartment.name.c_str());
        url = build_url(("storageLayers/"+ std::to_string(compartment.id) + "/childLayers").c_str());
        cJSON* compartment_columns = get_json(url.c_str());
        cJSON* column_arr = cJSON_GetObjectItemCaseSensitive(compartment_columns, "data");
        cJSON* column = NULL;
        std::vector<LayerItem> columns;
        cJSON_ArrayForEach(column, column_arr) {
            int column_id = cJSON_GetObjectItemCaseSensitive(column, "storageLayerID")->valueint;
            std::string column_name = cJSON_GetObjectItemCaseSensitive(column, "name")->valuestring;
            LayerItem item(column_id, column_name, 0);
            columns.push_back(item);
            
        }
        cJSON_Delete(compartment_columns);
        idx_col = 0;
        // loop over columns to yield drawer
        std::sort(columns.begin(), columns.end(), compare_layeritem_by_name); // sort alphabetically
        for (LayerItem& column: columns) {
            //ESP_LOGI(TAG, "Column: %s", column.name.c_str());
            column.ctr = idx_col;
            idx_col++;
            if (idx_col > max_idx_col) {max_idx_col = idx_col;}
            url = build_url(("storageLayers/"+ std::to_string(column.id) + "/childLayers").c_str());
            cJSON* compartment_drawers = get_json(url.c_str());
            cJSON* drawer_arr = cJSON_GetObjectItemCaseSensitive(compartment_drawers, "data");
            cJSON* drawer = NULL;
            std::vector<LayerItem> drawers;
            cJSON_ArrayForEach(drawer, drawer_arr) {
                int drawer_id = cJSON_GetObjectItemCaseSensitive(drawer, "storageLayerID")->valueint;
                std::string drawer_name = cJSON_GetObjectItemCaseSensitive(drawer, "name")->valuestring;
                LayerItem item(drawer_id, drawer_name, 0);
                drawers.push_back(item);
            }
            cJSON_Delete(compartment_drawers);

            // loop over drawers to yield boxes
            idx_row = 0;
            std::sort(drawers.begin(), drawers.end(), compare_layeritem_by_name); // sort alphabetically
            for (LayerItem& drawer: drawers) {
                //ESP_LOGI(TAG, "Drawer: '%s'", drawer.name.c_str());
                drawer.ctr = idx_row;
                idx_row++;
                if (idx_row > max_idx_row) {max_idx_row = idx_row;}
                url = build_url(("storageLayers/"+ std::to_string(drawer.id) + "/childLayers").c_str());
                cJSON* compartment_boxes = get_json(url.c_str());
                cJSON* boxes_arr = cJSON_GetObjectItemCaseSensitive(compartment_boxes, "data");
                cJSON* box = NULL;
                std::vector<LayerItem> boxes;
                cJSON_ArrayForEach(box, boxes_arr) {
                    int box_id = cJSON_GetObjectItemCaseSensitive(box, "storageLayerID")->valueint;
                    std::string box_name = cJSON_GetObjectItemCaseSensitive(box, "name")->valuestring;
                    //ESP_LOGI(TAG, "Box: %s", box_name.c_str());
                    LayerItem item(box_id, box_name);
                    boxes.push_back(item);
                }
                cJSON_Delete(compartment_boxes);

                // also sort alphabetically for consistency between API calls; not essential
                std::sort(boxes.begin(), boxes.end(), compare_layeritem_by_name);  
                // TODO handle the case of only a single box
                std::tuple boxpair{
                    FreezerBox{boxes[0].name, boxes[0].id, compartment.ctr, column.ctr, drawer.ctr},
                    FreezerBox{boxes[1].name, boxes[1].id, compartment.ctr, column.ctr, drawer.ctr}
                };

                content.drawers.push_back(boxpair);
                content.dimensions = {idx_comp, max_idx_col, max_idx_row};
            }
        }
    }
   
    return content;
}


// a function for testing and iterating on the UI (should really use the simulator for that)
FreezerAPI::FreezerContent FreezerAPI::get_dummy_freezer(){

    std::vector<std::tuple<FreezerBox, FreezerBox>> box_vec;  // intialise
    FreezerContent dummy_content("DUMMY FREEZER", box_vec,
        std::chrono::system_clock::now(),
        {2, 3, 5});

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 5; ++k) {
                std::tuple boxpair{
                    FreezerBox{"Dummy | apples, pears, cocaine", 1235566, i, j, k},
                    FreezerBox{"Dummy | apples, pears, cocaine", 1235566, i, j, k}
                };
                dummy_content.drawers.push_back(boxpair);
            }
        }
    } 

    return dummy_content;
}

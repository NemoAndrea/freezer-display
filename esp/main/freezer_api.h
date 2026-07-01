#pragma once

#include <string>
#include "cJSON.h"
#include <chrono>  


// helper function
inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}


class FreezerAPI {
public:
    struct FreezerBox{
        std::string owner;
        std::vector<std::string> samples;
        int storageLayerID;
        int compartment_idx;
        int column_idx;
        int row_idx;  

        // constructor
        FreezerBox(std::string raw_label, int storageLayerID, int compartment_idx,
             int column_idx, int row_idx)
             : owner("Error"),
               samples({}),
               storageLayerID(storageLayerID),
               compartment_idx(compartment_idx),
               column_idx(column_idx),
               row_idx(row_idx)
        {
            // parse the raw_label into owner and samples list
            // format is "owner | sample1, sample2, sample3"
            size_t pipePos = raw_label.find('|');
            owner = trim(raw_label.substr(0, pipePos));
            std::string samples_string = raw_label.substr(pipePos + 1);

            std::stringstream ss(samples_string);
            std::string item;
            
            while (std::getline(ss, item, ',')) {
                samples.push_back(trim(item));
            }
        }
    };

    struct FreezerContent{
        std::string label;
        std::vector<std::tuple<FreezerBox, FreezerBox>> drawers;
        std::chrono::system_clock::time_point time_retrieved;
        std::tuple<int, int, int> dimensions;
    };

    // constructor
    FreezerAPI(const char* url, const char* auth_token, const int freezerID): _endpoint(url), _token(auth_token), _freezer_id(freezerID) {}
    bool endpoint_available();
    FreezerContent get_freezer_content();
    FreezerContent get_dummy_freezer();

    FreezerContent contents;  // get the contents of the freezer

private:
    const char* _endpoint;
    const char* _token;
    const int _freezer_id;

    std::pair<esp_http_client_handle_t, esp_err_t> get_API_response(esp_http_client_config_t config, int timeout_ms);
    std::string build_url(const char* sub_path) const;
    cJSON* get_json(const char* url);
};



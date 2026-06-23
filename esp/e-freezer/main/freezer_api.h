#include <string>
#include "cJSON.h"
#include <chrono>  

class FreezerAPI {
public:
    struct FreezerBox{
        std::string label;
        int storageLayerID;
        int compartment_idx;
        int column_idx;
        int row_idx;  
    };

    struct FreezerContent{
        std::string label;
        std::vector<FreezerBox> boxes;
        std::chrono::system_clock::time_point time_retrieved;
        std::tuple<int, int, int> dimensions;
    };

    // constructor
    FreezerAPI(const char* url, const char* auth_token, const int freezerID): _endpoint(url), _token(auth_token), _freezer_id(freezerID) {}
    bool endpoint_available();
    FreezerContent get_freezer_content();

    FreezerContent contents;  // get the contents of the freezer

private:
    const char* _endpoint;
    const char* _token;
    const int _freezer_id;

    std::pair<esp_http_client_handle_t, esp_err_t> get_API_response(esp_http_client_config_t config, int timeout_ms);
    std::string build_url(const char* sub_path) const;
    cJSON* get_json(const char* url);
};
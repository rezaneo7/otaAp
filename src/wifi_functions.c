#include "headers.h"




char buf[256];
int otaProgress = 0;

static esp_err_t echo_post_handler(httpd_req_t* req){
 esp_err_t err;
esp_ota_handle_t update_handle = 0;
otaProgress = 1;
    const esp_partition_t *update_partition = NULL;
    ESP_LOGI(SERVER_TAG, "Starting OTA...");
    update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        ESP_LOGE(SERVER_TAG, "Passive OTA partition not found");
        return ESP_FAIL;
    }
    ESP_LOGI(SERVER_TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(SERVER_TAG, "esp_ota_begin failed, error=%d", err);
        return err;
    }
    ESP_LOGI(SERVER_TAG, "esp_ota_begin succeeded");
    ESP_LOGI(SERVER_TAG, "Please Wait. This may take time");

    esp_err_t ota_write_err = ESP_OK;
    char *upgrade_data_buf = (char *)malloc(OTA_BUF_SIZE);
    if (!upgrade_data_buf) {
        ESP_LOGE(SERVER_TAG, "Couldn't allocate memory to upgrade data buffer");
        return ESP_ERR_NO_MEM;
    }
    int binary_file_len = 0;
    char c[1];
    do{
    httpd_req_recv(req, c,1);

    }while(c[0]!=0xe9);
    ota_write_err = esp_ota_write( update_handle, c, 1);
    while (1) {
        int data_read = httpd_req_recv(req, buf,OTA_BUF_SIZE);
        
        if (data_read == 0) {
            ESP_LOGI(SERVER_TAG, "Connection closed,all data received");
            break;
        }
        if (data_read < 0) {
            ESP_LOGE(SERVER_TAG, "Error: SSL data read error");
            break;
        }
        if (data_read > 0 ) {
          

            ota_write_err = esp_ota_write( update_handle, buf, data_read);
            if (ota_write_err != ESP_OK) {
                break;
            }
            binary_file_len += data_read;
            ESP_LOGI(SERVER_TAG, "Written image length %d", binary_file_len);
           
           
        }

        //httpd_resp_send_chunk(req, buf, 256);
        
        
    }
    free(upgrade_data_buf);
    httpd_resp_send_chunk(req, "Done !!!", 8);
    httpd_resp_send_chunk(req, NULL, 0);
    ESP_LOGD(SERVER_TAG, "Total binary data length writen: %d", binary_file_len);
    
    esp_err_t ota_end_err = esp_ota_end(update_handle);
    if (ota_write_err != ESP_OK) {
        ESP_LOGE(SERVER_TAG, "Error: esp_ota_write failed! err=0x%d", err);
        return ota_write_err;
    } else if (ota_end_err != ESP_OK) {
        ESP_LOGE(SERVER_TAG, "Error: esp_ota_end failed! err=0x%d. Image is invalid", ota_end_err);
        return ota_end_err;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(SERVER_TAG, "esp_ota_set_boot_partition failed! err=0x%d", err);
        return err;
    }
    ESP_LOGI(SERVER_TAG, "esp_ota_set_boot_partition succeeded"); 
    otaProgress=0;
    esp_restart();
    return ESP_OK;



}




static const httpd_uri_t echo = {
    .uri       = "/update",
    .method    = HTTP_POST,
    .handler   = echo_post_handler,
    .user_ctx  = NULL
};





static esp_err_t file_get_handler(httpd_req_t *req)
{
    
    
    const char resp_str[] = "<!DOCTYPE html><html><body> <form action=\"/update\"   spellcheck=\"false\" method=\"POST\" enctype=\"multipart/form-data\">            <input type=\"file\" name=\"fileToUpload\" id=\"fileToUpload\">      <input type=\"submit\" value=\"Submit\"></from></body></html>";
    httpd_resp_send(req, resp_str, strlen(resp_str));

   
  
    return ESP_OK;
}

static const httpd_uri_t file = {
    .uri       = "/file",
    .method    = HTTP_GET,
    .handler   = file_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};

 httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(SERVER_TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(SERVER_TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &file);
        httpd_register_uri_handler(server, &echo);
       
        
        return server;
    }

    ESP_LOGI(SERVER_TAG, "Error starting server!");
    return NULL;
}

// Wifi event handler
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	tcpip_adapter_ip_info_t ip;
    ESP_LOGI(WIFI_TAG,"EVENT ID :%d",event->event_id);

    if(otaProgress==0){
    switch(event->event_id) {

        
	case SYSTEM_EVENT_AP_START:
       start_webserver();
      break;
	case SYSTEM_EVENT_AP_STACONNECTED:
            ESP_LOGI(WIFI_TAG, "station:"MACSTR"join",
                 MAC2STR(event->event_info.sta_connected.mac));
                            tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ip);    
                ESP_LOGI(WIFI_TAG, "SoftAP IP=%s", inet_ntoa(ip.ip.addr));
                
        
        break;
 
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(WIFI_TAG, "station:"MACSTR"leave",
                 MAC2STR(event->event_info.sta_disconnected.mac));
        break;	
    
    

	default:
        break;
    }

    }
   
	return ESP_OK;
}

void wifi_initialize(void) {
			
  
	
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    wifi_config_t wifi_config_ap = {

         .ap = {
            .ssid = SSID,
            .ssid_len = 7,
            .password =  "1020304050",
            .max_connection = 5,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config_ap));
	ESP_ERROR_CHECK(esp_wifi_start());
 
}


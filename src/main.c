#include "headers.h"




void app_main(){
    
    
	 esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }


   wifi_initialize();
   
  
    
   

}
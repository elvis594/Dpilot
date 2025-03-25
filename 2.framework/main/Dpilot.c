#include <stdio.h>
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO    9      // ESP32-S3 WROOM 推荐 SCL (可改)
#define I2C_MASTER_SDA_IO    8      // ESP32-S3 WROOM 推荐 SDA (可改)
#define I2C_MASTER_NUM       I2C_NUM_0  // I2C 端口
#define I2C_MASTER_FREQ_HZ   50000  // I2C 频率 100kHz
#define I2C_TIMEOUT_MS       1000    // 超时时间

static const char *TAG = "I2C_SCAN";

// 初始化 I2C
static esp_err_t i2c_master_init() {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// 扫描 I2C 设备
static void i2c_scan() {
    ESP_LOGI(TAG, "开始扫描 I2C 设备...");

    for (uint8_t addr = 0x03; addr < 0x78; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_TIMEOUT_MS / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "发现 I2C 设备: 0x%02X", addr);
        }
    }

    ESP_LOGI(TAG, "I2C 扫描完成！");
}

void app_main(void) {
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C 初始化完成");
    
    while (1) {
        i2c_scan();
        vTaskDelay(pdMS_TO_TICKS(5000));  // 每 5 秒扫描一次
    }
}

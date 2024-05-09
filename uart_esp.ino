#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)


// Insert your network credentials
#define WIFI_SSID "Giangvien"
#define WIFI_PASSWORD "dhbk@2024"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBnUEYApFonmRwA6us3YVOLiO7dZsOSMhA"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://gpsdata-9819f-default-rtdb.asia-southeast1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

float  longitude, latitude;
unsigned long sendDataPrevMillis = 0;
String timeg = "GP";
unsigned long t1 = 0;
int count = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  const uart_port_t uart_num = UART_NUM_2;
  uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
        .rx_flow_ctrl_thresh = 122,
  };
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, 18, 19));
    const int uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;
    // Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, uart_buffer_size, \
                                        uart_buffer_size, 20, &uart_queue, 0));
}
void loop() {
    // Read data from UART.
    const uart_port_t uart_num = UART_NUM_2;
    uint8_t data[128];
    int length = 0;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, (size_t*)&length));
    length = uart_read_bytes(uart_num, data, length, 100);
    if (length > 0) {
        data[length] = 0; // Null-terminate the data to make it a valid string
        char *test = (char*)data;
        //Serial.println(test);

        // Use sscanf to parse the string
        if (sscanf(test, "%f %f", &latitude, &longitude) == 2) {  // Ensure all three elements were read
            Serial.print("Latitude: ");
            Serial.println(latitude);  // Should print "Hello"
            Serial.print("Longitude: ");
            Serial.println(longitude);  // Should print 123

        } else {
            Serial.println("Error parsing string");
        }
    }
    //delay(1000); // Delay to prevent flooding
     if ( millis() - t1 > 1000)
  {
    //Firebase.RTDB.setString(&fbdo, "/GPS_Data/Time", timeg);
   // timeg = timeg + "SS";
    Firebase.RTDB.setFloat(&fbdo, "/GPS_Data/Latitude", latitude);
    Firebase.RTDB.setFloat(&fbdo, "/GPS_Data/Longitude", longitude);
  
    t1 = millis();
  }

}
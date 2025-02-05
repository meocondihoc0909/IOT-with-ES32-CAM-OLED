#include "esp_camera.h" // Nhúng thư viện để sử dụng camera trên ESP32
#include <WiFi.h> // Nhúng thư viện để kết nối và quản lý kết nối WiFi
#include <WebServer.h> // Nhúng thư viện để tạo máy chủ web trên ESP32
#include <Wire.h> // Nhúng thư viện để sử dụng giao tiếp I2C
#include <Adafruit_GFX.h> // Nhúng thư viện để sử dụng các chức năng đồ họa cơ bản cho màn hình
#include <Adafruit_SH110X.h> // Nhúng thư viện để điều khiển màn hình OLED SH110X

// Cấu hình chân GPIO cho CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32 // Chân GPIO để tắt nguồn camera
#define RESET_GPIO_NUM    -1 // Chân GPIO để reset camera (không sử dụng, -1 có nghĩa là không có chân)
#define XCLK_GPIO_NUM      0 // Chân GPIO cho tín hiệu xclk (tín hiệu đồng hồ)
#define SIOD_GPIO_NUM     26 // Chân GPIO cho tín hiệu dữ liệu I2C (SDA)
#define SIOC_GPIO_NUM     27 // Chân GPIO cho tín hiệu đồng hồ I2C (SCL)
#define Y9_GPIO_NUM       35 // Chân GPIO cho dữ liệu Y9 (dữ liệu hình ảnh từ camera)
#define Y8_GPIO_NUM       34 // Chân GPIO cho dữ liệu Y8
#define Y7_GPIO_NUM       39 // Chân GPIO cho dữ liệu Y7
#define Y6_GPIO_NUM       36 // Chân GPIO cho dữ liệu Y6
#define Y5_GPIO_NUM       21 // Chân GPIO cho dữ liệu Y5
#define Y4_GPIO_NUM       19 // Chân GPIO cho dữ liệu Y4
#define Y3_GPIO_NUM       18 // Chân GPIO cho dữ liệu Y3
#define Y2_GPIO_NUM        5 // Chân GPIO cho dữ liệu Y2
#define VSYNC_GPIO_NUM    25 // Chân GPIO cho tín hiệu VSYNC (tín hiệu đồng bộ dọc)
#define HREF_GPIO_NUM     23 // Chân GPIO cho tín hiệu HREF (tín hiệu đồng bộ ngang)
#define PCLK_GPIO_NUM     22 // Chân GPIO cho tín hiệu PCLK (tín hiệu đồng hồ pixel)

// Cấu hình màn hình OLED
#define SCREEN_WIDTH 128 // Chiều rộng màn hình OLED, tính bằng pixel
#define SCREEN_HEIGHT 64 // Chiều cao màn hình OLED, tính bằng pixel
#define OLED_RESET -1    // Chân reset cho OLED (hoặc -1 nếu chia sẻ chân reset của Arduino)
#define SDA_PIN 14 // Chân SDA cho giao tiếp I2C
#define SCK_PIN 15 // Chân SCL cho giao tiếp I2C

#define BUZZER_PIN 12   // Chân GPIO kết nối với buzzer
#define LED_PIN 13      // Chân GPIO kết nối với LED

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT); // Khởi tạo đối tượng màn hình OLED với kích thước đã định nghĩa

// Thông tin WiFi
const char* ssid = "Nguyn"; // Định nghĩa tên mạng WiFi
const char* password = "09092003"; // Định nghĩa mật khẩu mạng WiFi

WebServer server(80); // Khởi tạo máy chủ web trên cổng 80

String currentId = "None"; // Biến lưu trữ ID hiện tại
unsigned long lastUpdateTime = 0; // Thời gian cập nhật cuối cùng
const unsigned long updateInterval = 5000; // Thời gian cập nhật (5 giây)

void setup() {
  Serial.begin(115200); // Khởi tạo giao tiếp nối tiếp với tốc độ 115200 bps
  Wire.begin(SDA_PIN, SCK_PIN); // Bắt đầu giao tiếp I2C với chân SDA và SCL đã được định nghĩa
  display.begin(); // Bắt đầu giao tiếp với màn hình OLED
  display.clearDisplay(); // Xóa nội dung màn hình OLED
  display.setTextSize(1); // Thiết lập kích thước chữ là 1
  display.setTextColor(SH110X_WHITE); // Thiết lập màu chữ là trắng
  display.setCursor(10, 0); // Đặt con trỏ ở vị trí (0, 0) trên màn hình
  display.println("ESP32-CAM"); // In dòng chữ " ESP32-CAM" lên màn hình
  display.println(WiFi.localIP()); // In địa chỉ IP của ESP32 lên màn hình
  display.print("ID: "); // In "ID: " lên màn hình
  display.println(currentId); // In ID hiện tại lên màn hình
  display.display(); // Cập nhật màn hình để hiển thị nội dung đã in

  pinMode(BUZZER_PIN, OUTPUT); // Đặt chân GPIO cho buzzer là đầu ra
  pinMode(LED_PIN, OUTPUT); // Đặt chân GPIO cho LED là đầu ra
  digitalWrite(LED_PIN, HIGH); // Bật LED

  camera_config_t config; // Khai báo cấu hình cho camera
  config.ledc_channel = LEDC_CHANNEL_0; // Chọn kênh LEDC cho camera
  config.ledc_timer = LEDC_TIMER_0; // Chọn timer LEDC cho camera
  config.pin_d0 = Y2_GPIO_NUM; // Gán chân dữ liệu D0
  config.pin_d1 = Y3_GPIO_NUM; // Gán chân dữ liệu D1
  config.pin_d2 = Y4_GPIO_NUM; // Gán chân dữ liệu D2
  config.pin_d3 = Y5_GPIO_NUM; // Gán chân dữ liệu D3
  config.pin_d4 = Y6_GPIO_NUM; // Gán chân dữ liệu D4
  config.pin_d5 = Y7_GPIO_NUM; // Gán chân dữ liệu D5
  config.pin_d6 = Y8_GPIO_NUM; // Gán chân dữ liệu D6
  config.pin_d7 = Y9_GPIO_NUM; // Gán chân dữ liệu D7
  config.pin_xclk = XCLK_GPIO_NUM; // Gán chân xclk
  config.pin_pclk = PCLK_GPIO_NUM; // Gán chân pclk
  config.pin_vsync = VSYNC_GPIO_NUM; // Gán chân vsync
  config.pin_href = HREF_GPIO_NUM; // Gán chân href
  config.pin_sscb_sda = SIOD_GPIO_NUM; // Gán chân SDA cho giao tiếp I2C
  config.pin_sscb_scl = SIOC_GPIO_NUM; // Gán chân SCL cho giao tiếp I2C
  config.pin_pwdn = PWDN_GPIO_NUM; // Gán chân pwdn
  config.pin_reset = RESET_GPIO_NUM; // Gán chân reset
  config.xclk_freq_hz = 20000000; // Tần số xclk là 20 MHz
  config.pixel_format = PIXFORMAT_JPEG; // Định dạng pixel là JPEG

  if(psramFound()){ // Kiểm tra xem PSRAM có tồn tại không
      config.frame_size = FRAMESIZE_VGA; // Nếu có, thiết lập kích thước khung hình là VGA
      config.jpeg_quality = 10; // Chất lượng JPEG là 10
      config.fb_count = 2; // Số lượng buffer là 2
  } else {
      config.frame_size = FRAMESIZE_SVGA; // Nếu không có, thiết lập kích thước khung hình là SVGA
      config.jpeg_quality = 12; // Chất lượng JPEG là 12
      config.fb_count = 1; // Số lượng buffer là 1
  }

  esp_err_t err = esp_camera_init(&config); // Khởi tạo camera với cấu hình đã định nghĩa
  if (err != ESP_OK) { // Kiểm tra xem khởi tạo có thành công không
      Serial.printf("Khởi tạo Camera thất bại: 0x%x", err); // In ra lỗi nếu khởi tạo thất bại
      return; // Thoát hàm setup
  }

  WiFi.begin(ssid, password); // Bắt đầu kết nối WiFi với SSID và mật khẩu đã định nghĩa
  while (WiFi.status() != WL_CONNECTED) { // Chờ cho đến khi kết nối WiFi thành công
    delay(500); // Tạm dừng 500ms
    Serial.print("."); // In dấu chấm để hiển thị quá trình kết nối
  }
  Serial.println(""); // Xuống dòng
  Serial.println("WiFi đã kết nối"); // In thông báo kết nối WiFi thành công
  Serial.println("Địa chỉ IP: "); // In thông báo địa chỉ IP
  Serial.println(WiFi.localIP()); // In địa chỉ IP của ESP32

  display.clearDisplay(); // Xóa nội dung màn hình OLED
  display.setCursor(10, 0); // Đặt con trỏ ở vị trí (0, 0) trên màn hình
  display.println("ESP32-CAM"); // In dòng chữ "ESP32-CAM" lên màn hình
  display.println(WiFi.localIP()); // In địa chỉ IP của ESP32 lên màn hình
  display.print("ID: "); // In "ID: " lên màn hình
  display.println(currentId); // In ID hiện tại lên màn hình
  display.display(); // Cập nhật màn hình để hiển thị nội dung đã in
  server.on("/cam-mid.jpg", HTTP_GET, handleJpgMid); // Đăng ký đường dẫn "/cam-mid.jpg" để xử lý yêu cầu GET bằng hàm handleJpgMid
  server.on("/send-id", HTTP_GET, handleSendId); // Đăng ký đường dẫn "/send-id" để xử lý yêu cầu GET bằng hàm handleSendId

  server.begin(); // Bắt đầu máy chủ web
}

void loop() {
  server.handleClient(); // Xử lý các yêu cầu từ khách hàng

  // Kiểm tra thời gian để cập nhật màn hình OLED
  unsigned long currentMillis = millis(); // Lấy thời gian hiện tại
  if (currentMillis - lastUpdateTime >= updateInterval) { // Kiểm tra xem đã đến thời gian cập nhật chưa
    lastUpdateTime = currentMillis; // Cập nhật thời gian cuối cùng
    // Cập nhật màn hình OLED với ID hiện tại
    display.clearDisplay(); // Xóa nội dung màn hình OLED
    display.setCursor(10, 0); // Đặt con trỏ ở vị trí (0, 0) trên màn hình
    display.println("ESP32-CAM"); // In dòng chữ "ESP32-CAM" lên màn hình
    display.println(WiFi.localIP()); // In địa chỉ IP của ESP32 lên màn hình
    display.print("ID: "); // In "ID: " lên màn hình
    display.println(currentId); // In ID hiện tại lên màn hình
    display.display(); // Cập nhật màn hình để hiển thị nội dung đã in
  }
}

void handleJpgMid() { // Hàm xử lý yêu cầu chụp ảnh
  camera_fb_t * fb = NULL; // Khai báo con trỏ cho buffer hình ảnh
  fb = esp_camera_fb_get(); // Lấy buffer hình ảnh từ camera
  if (!fb) { // Kiểm tra xem có lấy được buffer không
    Serial.println("Không thể chụp ảnh"); // In thông báo lỗi nếu không chụp được ảnh
    server.send(500, "text/plain", "Lỗi Camera"); // Gửi phản hồi lỗi cho khách hàng
    return; // Thoát hàm
  }
  
  server.sendHeader("Content-Type", "image/jpeg"); // Gửi header cho loại nội dung là hình ảnh JPEG
  server.sendHeader("Content-Length", String(fb->len)); // Gửi header cho chiều dài nội dung
  server.sendHeader("Content-Disposition", "inline; filename=capture.jpg"); // Gửi header cho tên tệp
  server.sendHeader("Access-Control-Allow-Origin", "*"); // Cho phép truy cập từ mọi nguồn
  server.send_P(200, "image/jpeg", (const char *)fb->buf, fb->len); // Gửi hình ảnh cho khách hàng
  
  esp_camera_fb_return(fb); // Trả lại buffer hình ảnh cho camera
}

void handleSendId() { // Hàm xử lý yêu cầu gửi ID
  if(!server.hasArg("id")) { // Kiểm tra xem có tham số "id" không
    server.send(400, "text/plain", "Thiếu tham số id"); // Gửi phản hồi lỗi nếu thiếu tham số
    return; // Thoát hàm
  }
  
  String id = server.arg("id"); // Lấy giá trị của tham số "id"
  Serial.println("Nhận ID: " + id); // In ID nhận được ra màn hình

  // Phát tiếng bip từ buzzer
  digitalWrite(BUZZER_PIN, HIGH); // Bật buzzer
  delay(1000); // Đợi 1 giây
  digitalWrite(BUZZER_PIN, LOW); // Tắt buzzer

  // Cập nhật ID hiện tại
  currentId = id; // Lưu ID mới nhận được
  server.send(200, "text/plain", "OK"); // Gửi phản hồi thành công cho khách hàng
}
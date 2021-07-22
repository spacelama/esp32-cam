/*
  NTP-TZ-DST (v2)
  NetWork Time Protocol - Time Zone - Daylight Saving Time

  This example shows:
  - how to read and set time
  - how to set timezone per country/city
  - how is local time automatically handled per official timezone definitions
  - how to change internal sntp start and update delay
  - how to use callbacks when time is updated

  This example code is in the public domain.
*/

// This database is autogenerated from IANA timezone database
//    https://www.iana.org/time-zones
// and can be updated on demand in this repository
#include <time.h>
#include <WiFi.h>
#include "template.h"
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
//#include <sntp.h>                       // sntp_servermode_dhcp()
//#include <coredecls.h>                  // settimeofday_cb()
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <RTClib.h>
#include "Adafruit_LEDBackpack.h"
#include <WebServer.h>
#include "esp_camera.h"
#include <Syslog.h>

extern Syslog syslog;


//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include "camera_pins.h"

// from app_httpd.cpp:
#include "esp_camera.h"
#include "img_converters.h"
#include "camera_index.h"
#include "fb_gfx.h"

/* typedef struct { */
/*         httpd_req_t *req; */
/*         size_t len; */
/* } jpg_chunking_t; */

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

String CODE_VERSION = "$Revision: 1.9 $";
WebServer server(80);

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
//    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
//        j->len = 0;
    }
//    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
//        return 0;
//    }
//    j->len += len;
    return len;
}

void capture_handler(){//httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    //  int64_t fr_start = esp_timer_get_time();

    // flash led
//    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);

    // cause a full readout, which might be available immediately, and
    // hopefully calling it again then causes a wait on the next
    // frame, which will then be fully illuminated by the flash
//    fb = esp_camera_fb_get();
//    esp_camera_fb_return(fb);
    delay(300);

    fb = esp_camera_fb_get();
    digitalWrite(4, LOW);
    if (!fb) {
        Serial.println("Camera capture failed");
//        httpd_resp_send_500(req);
        server.send(500, "Camera capture failed");
        return;
    }

    /* httpd_resp_set_type(req, "image/jpeg"); */
    /* httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg"); */
    /* httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"); */
    server.sendHeader("Content-Disposition", "inline; filename=capture.jpg", true);
    server.sendHeader("Access-Control-Allow-Origin", "*", false);

    size_t out_len, out_width, out_height;
    uint8_t * out_buf;
    bool s;
    int face_id = 0;
    size_t fb_len = 0;

//first create a fixed buffer
    const int bufferSize = 6000;
    char _buffer[6000];

//a little counter to know at which position we are in our buffer
    int bufferCounter = 0;

    if(fb->format == PIXFORMAT_JPEG){
        fb_len = fb->len;
//            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        // https://gist.github.com/spacehuhn/6c89594ad0edbdb0aad60541b72b2388
//        server.sendHeader("Content-Length", (String)fb->len);
        server.setContentLength(fb->len);
//        server.sendHeader("Content-Type", "image/jpeg", false);
        server.send(200, "image/jpeg", "");
        for(int i=0;i<fb->len;i++){
            _buffer[bufferCounter]=fb->buf[i]; // copy the framebuffer into our temporary buffer
            bufferCounter++;

            if(bufferCounter >= bufferSize){ //when the buffer is full...
                server.sendContent_P(_buffer, bufferCounter); //send the current buffer
                bufferCounter = 0; //reset the counter
            }
        }
        //send the rest bytes if there are some
        if(bufferCounter > 0){
            server.sendContent_P(_buffer, bufferCounter);
            bufferCounter = 0;
        }
        syslog.log(LOG_INFO, "esp32cam/capture");
    } else {
//            jpg_chunking_t jchunk = {req, 0};
//            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
//            httpd_resp_send_chunk(req, NULL, 0);
//            fb_len = jchunk.len;
    }
    esp_camera_fb_return(fb);
//        int64_t fr_end = esp_timer_get_time();
//        Serial.printf("JPG: %uB %ums\n", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start)/1000));
}


void stream_handler() { //httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char part_buf[64];

//  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
//  if(res != ESP_OK){
//    return res;
//  }
    server.send(200, _STREAM_CONTENT_TYPE, "");

    while(true){
        // flash led
//    pinMode(4, OUTPUT);
        digitalWrite(4, HIGH);
        // cause a full readout, which might be available immediately, and
        // hopefully calling it again then causes a wait on the next
        // frame, which will then be fully illuminated by the flash
//    fb = esp_camera_fb_get();
//    esp_camera_fb_return(fb);
        delay(300);

        fb = esp_camera_fb_get();
        digitalWrite(4, LOW);
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
            if(fb->width > 400){
                if(fb->format != PIXFORMAT_JPEG){
                    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                    esp_camera_fb_return(fb);
                    fb = NULL;
                    if(!jpeg_converted){
                        Serial.println("JPEG compression failed");
                        res = ESP_FAIL;
                    }
                } else {
                    _jpg_buf_len = fb->len;
                    _jpg_buf = fb->buf;
                }
            }
        }
        /* WiFiClient client = server._server.available(); */
        /* if (client && !client.connected()) { */
        /*     res = ESP_FAIL; */
        /* } */
        //FIXME: must detect disconnected client
        if(res == ESP_OK){
            size_t hlen = snprintf(part_buf, 64, _STREAM_PART, _jpg_buf_len);
//      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
            server.sendContent_P(part_buf, hlen);
//    }
//    if(res == ESP_OK){
//      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
            server.sendContent_P((const char *)_jpg_buf, _jpg_buf_len);
//    }
//    if(res == ESP_OK){
//      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
            server.sendContent_P(_STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        syslog.log(LOG_INFO, "esp32cam/stream(cont)");
        if(fb){
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if(_jpg_buf){
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if(res != ESP_OK){
            break;
        }
        //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
    }
    return;
}


void cmd_handler(){//httpd_req_t *req){
    /* char*  buf; */
    /* size_t buf_len; */
    /* char variable[32] = {0,}; */
    /* char value[32] = {0,}; */

/* //    buf_len = httpd_req_get_url_query_len(req) + 1; */
/*     if (buf_len > 1) { */
/*         buf = (char*)malloc(buf_len); */
/*         if(!buf){ */
/* //            httpd_resp_send_500(req); */
/* //            return ESP_FAIL; */
/*             return; */
/*         } */
    /* if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) { */
    /*     if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK && */
    /*         httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) { */
    /*     } else { */
    /*         free(buf); */
    /*         httpd_resp_send_404(req); */
    /*         return ESP_FAIL; */
    /*     } */
    /* } else { */
    /*     free(buf); */
    /*     httpd_resp_send_404(req); */
    /*     return ESP_FAIL; */
    /* } */
/*         free(buf); */
/*     } else { */
/* //        httpd_resp_send_404(req); */
/* //        return ESP_FAIL; */
/*         return; */
/*     } */

    if (server.args() < 2) {
        server.send(500, "No commands to parse");
        return;
    }
    String variable = server.arg("var");
    String value    = server.arg("val");
    if (variable == "") {
        server.send(500, "No var supplied");
        return;
    }
    if (value == "") {
        server.send(500, "No val supplied");
        return;
    }

    syslog.logf(LOG_INFO, "esp32cam(%d): %s = %s", server.args(), variable.c_str(), value.c_str());

    int val = atoi(value.c_str());
    sensor_t * s = esp_camera_sensor_get();
    int res = 0;

    if(!strcmp(variable.c_str(), "framesize")) {
        if(s->pixformat == PIXFORMAT_JPEG) res = s->set_framesize(s, (framesize_t)val);
    }
    else if(!strcmp(variable.c_str(), "quality")) res = s->set_quality(s, val);
    else if(!strcmp(variable.c_str(), "contrast")) res = s->set_contrast(s, val);
    else if(!strcmp(variable.c_str(), "brightness")) res = s->set_brightness(s, val);
    else if(!strcmp(variable.c_str(), "saturation")) res = s->set_saturation(s, val);
    else if(!strcmp(variable.c_str(), "gainceiling")) res = s->set_gainceiling(s, (gainceiling_t)val);
    else if(!strcmp(variable.c_str(), "colorbar")) res = s->set_colorbar(s, val);
    else if(!strcmp(variable.c_str(), "awb")) res = s->set_whitebal(s, val);
    else if(!strcmp(variable.c_str(), "agc")) res = s->set_gain_ctrl(s, val);
    else if(!strcmp(variable.c_str(), "aec")) res = s->set_exposure_ctrl(s, val);
    else if(!strcmp(variable.c_str(), "hmirror")) res = s->set_hmirror(s, val);
    else if(!strcmp(variable.c_str(), "vflip")) res = s->set_vflip(s, val);
    else if(!strcmp(variable.c_str(), "awb_gain")) res = s->set_awb_gain(s, val);
    else if(!strcmp(variable.c_str(), "agc_gain")) res = s->set_agc_gain(s, val);
    else if(!strcmp(variable.c_str(), "aec_value")) res = s->set_aec_value(s, val);
    else if(!strcmp(variable.c_str(), "aec2")) res = s->set_aec2(s, val);
    else if(!strcmp(variable.c_str(), "dcw")) res = s->set_dcw(s, val);
    else if(!strcmp(variable.c_str(), "bpc")) res = s->set_bpc(s, val);
    else if(!strcmp(variable.c_str(), "wpc")) res = s->set_wpc(s, val);
    else if(!strcmp(variable.c_str(), "raw_gma")) res = s->set_raw_gma(s, val);
    else if(!strcmp(variable.c_str(), "lenc")) res = s->set_lenc(s, val);
    else if(!strcmp(variable.c_str(), "special_effect")) res = s->set_special_effect(s, val);
    else if(!strcmp(variable.c_str(), "wb_mode")) res = s->set_wb_mode(s, val);
    else if(!strcmp(variable.c_str(), "ae_level")) res = s->set_ae_level(s, val);
    else {
        res = -1;
    }

    if(res){
        //      return httpd_resp_send_500(req);
        server.send(500, "Command parse fail");
    }

//    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
//    return httpd_resp_send(req, NULL, 0);
    server.sendHeader("Access-Control-Allow-Origin", "*", true);
    server.send(200, "", "");
}

void status_handler(){//httpd_req_t *req){
    static char json_response[1024];

    sensor_t * s = esp_camera_sensor_get();
    char * p = json_response;
    *p++ = '{';

    p+=sprintf(p, "\"framesize\":%u,", s->status.framesize);
    p+=sprintf(p, "\"quality\":%u,", s->status.quality);
    p+=sprintf(p, "\"brightness\":%d,", s->status.brightness);
    p+=sprintf(p, "\"contrast\":%d,", s->status.contrast);
    p+=sprintf(p, "\"saturation\":%d,", s->status.saturation);
    p+=sprintf(p, "\"sharpness\":%d,", s->status.sharpness);
    p+=sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
    p+=sprintf(p, "\"wb_mode\":%u,", s->status.wb_mode);
    p+=sprintf(p, "\"awb\":%u,", s->status.awb);
    p+=sprintf(p, "\"awb_gain\":%u,", s->status.awb_gain);
    p+=sprintf(p, "\"aec\":%u,", s->status.aec);
    p+=sprintf(p, "\"aec2\":%u,", s->status.aec2);
    p+=sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
    p+=sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
    p+=sprintf(p, "\"agc\":%u,", s->status.agc);
    p+=sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
    p+=sprintf(p, "\"gainceiling\":%u,", s->status.gainceiling);
    p+=sprintf(p, "\"bpc\":%u,", s->status.bpc);
    p+=sprintf(p, "\"wpc\":%u,", s->status.wpc);
    p+=sprintf(p, "\"raw_gma\":%u,", s->status.raw_gma);
    p+=sprintf(p, "\"lenc\":%u,", s->status.lenc);
    p+=sprintf(p, "\"vflip\":%u,", s->status.vflip);
    p+=sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
    p+=sprintf(p, "\"dcw\":%u,", s->status.dcw);
    p+=sprintf(p, "\"colorbar\":%u,", s->status.colorbar);
    *p++ = '}';
    *p++ = 0;
//    httpd_resp_set_type(req, "application/json");
//    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
//    return httpd_resp_send(req, json_response, strlen(json_response));
    server.sendHeader("Access-Control-Allow-Origin", "*", true);
//    server.sendHeader("Content-Type", "application/json", false);
    server.send(200, "application/json", json_response);
}

void index_handler(){//httpd_req_t *req){
    /* httpd_resp_set_type(req, "text/html"); */
    /* httpd_resp_set_hdr(req, "Content-Encoding", "gzip"); */
    server.sendHeader("Content-Encoding", "gzip", true);
    sensor_t * s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {
//        return httpd_resp_send(req, (const char *)index_ov3660_html_gz, index_ov3660_html_gz_len);
        server.setContentLength(index_ov3660_html_gz_len);
        server.send(200, "text/html");
        server.sendContent_P(index_ov3660_html_gz, index_ov3660_html_gz_len);
    } else {
        server.setContentLength(index_ov2640_html_gz_len);
        server.send(200, "text/html");
        server.sendContent_P(index_ov2640_html_gz, index_ov2640_html_gz_len);
    }
//    return httpd_resp_send(req, (const char *)index_ov2640_html_gz, index_ov2640_html_gz_len);
}


void http_getindex() {
    index_handler();
}

void http_status() {
    status_handler();
}

void http_control() {
    cmd_handler();
}

void http_capture() {
    capture_handler();
}

void http_stream() {
    stream_handler();
}

String http_uptime_stub() {
    return "";
}

void http_start_stub() {
    server.on("/", http_getindex);
    server.on("/status", http_status);
    server.on("/control", http_control);
    server.on("/capture", http_capture);
    server.on("/stream", http_stream);

    server.enableDelay(true);
};

void setup_stub() {
    Serial.println();

    Serial.println("esp32cam");

    Serial.setDebugOutput(true);
    Serial.println();

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
  
    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
    //                      for larger pre-allocated frame buffer.
    if(psramFound()){
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }

#if defined(CAMERA_MODEL_ESP_EYE)
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
#endif

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t * s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1); // flip it back
        s->set_brightness(s, 1); // up the brightness just a bit
        s->set_saturation(s, -2); // lower the saturation
    }
    // drop down frame size for higher initial frame rate
    // Nah, not for the security camera implementation
//  s->set_framesize(s, FRAMESIZE_QVGA);
    s->set_framesize(s, FRAMESIZE_UXGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
#endif

    // led led:
    //pinMode(33, OUTPUT);
    //digitalWrite(33, LOW);
    // flash led
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);
    delay(50);
    digitalWrite(4, LOW);
}

void loop_stub() {
}

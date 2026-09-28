#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <lvgl.h>

TFT_eSPI tft = TFT_eSPI();

// Buffer de desenho do LVGL
static const uint32_t screenWidth  = 240;
static const uint32_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

// Elementos da UI
lv_obj_t * status_label;
lv_obj_t * avatar_label;
lv_obj_t * subtitle_label;

// Controle de Animação e Estados
bool is_recording = false;
bool is_speaking = false;
int avatar_frame = 0;
lv_timer_t * anim_timer;

// Pino do Botão
#define BUTTON_PIN 4

// ==========================================
// FUNÇÕES LVGL E DISPLAY
// ==========================================
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();
    lv_disp_flush_ready(disp_drv);
}

void build_ui() {
    lv_obj_t * screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0); // Fundo preto
    lv_obj_set_style_text_color(screen, lv_color_hex(0xFFFFFF), 0);

    // 1. Status Bar (Wi-Fi, Bateria, BT)
    status_label = lv_label_create(screen);
    lv_label_set_text(status_label, "WiFi: ON | BT: OFF | 95%");
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_12, 0);

    // 2. Avatar da IA (usando texto simulando um rosto por simplicidade)
    avatar_label = lv_label_create(screen);
    lv_label_set_text(avatar_label, "(o_o)"); 
    lv_obj_align(avatar_label, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_text_font(avatar_label, &lv_font_montserrat_32, 0);

    // 3. Legenda (Texto que desliza)
    subtitle_label = lv_label_create(screen);
    lv_label_set_long_mode(subtitle_label, LV_LABEL_LONG_SCROLL_CIRCULAR); 
    lv_obj_set_width(subtitle_label, 180);
    lv_label_set_text(subtitle_label, "Ola! Eu sou o Casaro, seu assistente. Aperte o botao para falar.");
    lv_obj_align(subtitle_label, LV_ALIGN_BOTTOM_MID, 0, -40);
    lv_obj_set_style_text_align(subtitle_label, LV_TEXT_ALIGN_CENTER, 0);
}

// Anima o avatar se estiver falando
void anim_timer_cb(lv_timer_t * timer) {
    if (is_recording) {
        lv_label_set_text(avatar_label, "(( GRAVANDO ))");
        lv_obj_set_style_text_color(avatar_label, lv_color_hex(0xFF0000), 0);
    } else if (is_speaking) {
        lv_obj_set_style_text_color(avatar_label, lv_color_hex(0x00FF00), 0);
        if (avatar_frame == 0) { lv_label_set_text(avatar_label, "(^O^)"); avatar_frame = 1; }
        else { lv_label_set_text(avatar_label, "(^-^)"); avatar_frame = 0; }
    } else {
        lv_label_set_text(avatar_label, "(o_o)");
        lv_obj_set_style_text_color(avatar_label, lv_color_hex(0xFFFFFF), 0);
    }
}

// ==========================================
// FUNÇÕES DE LÓGICA E CONECTIVIDADE
// ==========================================
void send_dummy_audio_to_backend() {
    lv_label_set_text(subtitle_label, "Processando na Nuvem...");
    
    // No Wokwi VS Code, o IP 10.0.2.2 acessa o localhost da sua máquina!
    String api_url = "http://10.0.2.2:8000/chat"; 
    
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(api_url);
        
        // Enviando um arquivo falso só pra testar a comunicação
        String boundary = "----ColarInteligenteBoundary";
        http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
        String payload = "--" + boundary + "\r\nContent-Disposition: form-data; name=\"audio\"; filename=\"test.webm\"\r\nContent-Type: application/octet-stream\r\n\r\nFALSO_AUDIO\r\n--" + boundary + "--\r\n";
        
        int httpResponseCode = http.POST(payload);
        
        if (httpResponseCode > 0) {
            String aiResponse = http.header("X-AI-Response");
            if(aiResponse.length() > 0) {
                // A API decodifica a URL
                aiResponse.replace("%20", " ");
                lv_label_set_text(subtitle_label, aiResponse.c_str());
                is_speaking = true; 
            } else {
                lv_label_set_text(subtitle_label, "Resposta recebida, mas sem texto.");
                is_speaking = true;
            }
        } else {
            lv_label_set_text(subtitle_label, "Erro ao conectar com API Groq.");
        }
        http.end();
        
        // Simular o tempo de fala
        delay(5000); 
        is_speaking = false;
        lv_label_set_text(subtitle_label, "Aguardando...");
    } else {
        lv_label_set_text(subtitle_label, "Sem Internet!");
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Configura o Display
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    // Inicializa LVGL
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);
    
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    build_ui();
    
    // Timer para atualizar avatar a cada 300ms
    anim_timer = lv_timer_create(anim_timer_cb, 300, NULL);

    // Conecta Wi-Fi do simulador Wokwi (SSID padrão "Wokwi-GUEST")
    WiFi.begin("Wokwi-GUEST", "", 6);
    lv_label_set_text(status_label, "WiFi: Conectando... | BT: OFF | 95%");
    
    // Deixa a tela renderizar enquanto conecta
    for(int i=0; i<10; i++){
        lv_task_handler();
        delay(10);
    }
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    lv_label_set_text(status_label, "WiFi: OK | BT: OFF | 95%");
}

bool last_btn_state = HIGH;

void loop() {
    lv_task_handler(); // Mantém a tela viva
    delay(5);

    // Lógica do botão (Pressionou -> Toggle Gravação)
    bool btn_state = digitalRead(BUTTON_PIN);
    if (btn_state == LOW && last_btn_state == HIGH) {
        // Debounce simples
        delay(50);
        if(digitalRead(BUTTON_PIN) == LOW){
            if (!is_recording) {
                is_recording = true;
                lv_label_set_text(subtitle_label, "Escutando...");
            } else {
                is_recording = false;
                send_dummy_audio_to_backend();
            }
        }
    }
    last_btn_state = btn_state;
}

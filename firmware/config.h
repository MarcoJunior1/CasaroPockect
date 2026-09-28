// ============================================================
// config.h — Configurações globais, pinos e constantes
// Casaro – Assistente de IA Vestível
// ============================================================
#pragma once

// ---- Wi-Fi (RF02) ----
#define WIFI_SSID       "NOME_DA_SUA_REDE"
#define WIFI_PASSWORD   "SENHA_DA_SUA_REDE"
#define WIFI_TIMEOUT_MS 15000

// ---- Backend (RF05, RF17, RF18) ----
// IMPORTANTE: A chave da API Groq fica APENAS no backend Python (RF50)
#define BACKEND_HOST    "http://192.168.1.X"  // IP do PC com o servidor Python
#define BACKEND_PORT    8000
#define BACKEND_URL     BACKEND_HOST ":8000/chat"

// ---- Pinos do Display GC9A01 (SPI) — RF25 ----
#define TFT_MOSI  11
#define TFT_SCLK  12
#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST    8
#define TFT_BL    46   // Backlight (RF44 — apaga por inatividade)
#define TFT_W    240
#define TFT_H    240

// ---- Pinos do Microfone INMP441 (I2S RX) — RF10 ----
#define MIC_I2S_PORT   I2S_NUM_0
#define MIC_BCK_PIN    40   // BCLK
#define MIC_WS_PIN     41   // LRCL / WS
#define MIC_SD_PIN     42   // DOUT (dados do microfone)
#define MIC_SAMPLE_RATE  16000
#define MIC_RECORD_SECS  5
#define MIC_BUFFER_BYTES (MIC_SAMPLE_RATE * 2 * MIC_RECORD_SECS)

// ---- Pinos do Alto-falante MAX98357A (I2S TX) — RF21, RF23 ----
#define SPK_I2S_PORT   I2S_NUM_1
#define SPK_BCK_PIN    35
#define SPK_WS_PIN     36
#define SPK_DATA_PIN   37
#define SPK_SHUTDOWN   38   // SD pin do MAX98357A (LOW = mudo)

// ---- MPU6050 Acelerômetro/Giroscópio (I2C) — RF30 ----
#define MPU_SDA_PIN     4
#define MPU_SCL_PIN     5
#define MPU_I2C_ADDR  0x68

// ---- Câmera (RF06) — XIAO ESP32-S3 Sense integrada ----
// Pinos padrão da câmera OV2640 no XIAO ESP32-S3 Sense
// (não alterar — são fixos no hardware)

// ---- Cartão microSD (SPI compartilhado) — RF38 ----
#define SD_CS_PIN      21

// ---- Bateria (RF41, RF43) ----
#define BAT_ADC_PIN     1    // GPIO1 / ADC1_CH0
#define BAT_FULL_MV  4200    // mV = bateria cheia
#define BAT_EMPTY_MV 3300    // mV = bateria vazia
#define BAT_DIVIDER   2.0f   // Divisor resistivo (R1=R2)

// ---- Botão principal (RF11) ----
#define BUTTON_PIN     0     // GPIO0 (BOOT button no DevKit, ou botão externo)

// ---- LED indicador de status (RF47) ----
#define LED_MIC_PIN    45    // Acende quando microfone está ativo
#define LED_CAM_PIN    48    // Acende quando câmera está ativa (RF46)

// ---- Economia de energia (RF43, RF44, RF45) ----
#define SCREEN_TIMEOUT_MS  30000   // Apaga tela após 30s de inatividade
#define WIFI_IDLE_TIMEOUT  60000   // Desativa WiFi após 60s sem uso
#define DEEP_SLEEP_TIMEOUT 300000  // Deep sleep após 5min de inatividade total

// ---- OTA (RF51) ----
#define OTA_HOSTNAME   "casaro-colar"
#define OTA_PASSWORD   "casaro123"

// ---- Modos de operação (RF04, RF20, RF28) ----
enum SystemMode {
    MODE_ONLINE  = 0,   // WiFi OK, usa backend + Groq
    MODE_OFFLINE = 1,   // Sem WiFi, respostas locais simples
    MODE_DEMO    = 2,   // Modo demonstração offline
    MODE_BLE     = 3    // BLE como bridge para o celular (RF36)
};

// ---- Estados internos ----
enum DeviceState {
    STATE_BOOT      = 0,
    STATE_IDLE      = 1,
    STATE_RECORDING = 2,
    STATE_WAITING   = 3,
    STATE_SPEAKING  = 4,
    STATE_SLEEP     = 5
};

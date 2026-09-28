#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/i2s.h>

// ==========================================
// 1. CONFIGURAÇÕES DE REDE E API
// ==========================================
const char* ssid = "NOME_DA_SUA_REDE";
const char* password = "SENHA_DA_SUA_REDE";

// Substitua pelo IP local da sua máquina rodando o Python (ex: 192.168.1.X)
String api_url = "http://192.168.1.X:8000/chat"; 

// ==========================================
// 2. PINOS DO MICROFONE INMP441 (I2S)
// Ajuste de acordo com a pinagem usada no XIAO
// ==========================================
#define I2S_MIC_PORT I2S_NUM_0
#define I2S_MIC_WS   42 // LRCL
#define I2S_MIC_SD   41 // DOUT
#define I2S_MIC_SCK  40 // BCLK

// ==========================================
// 3. PARÂMETROS DE GRAVAÇÃO
// ==========================================
#define SAMPLE_RATE 16000
#define RECORD_TIME_SECONDS 5 // Grava 5 segundos de áudio por vez
const size_t RECORD_BUFFER_SIZE = SAMPLE_RATE * 2 * RECORD_TIME_SECONDS; // 16-bit = 2 bytes
uint8_t* audio_buffer = nullptr;

void setup_wifi() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado! IP: " + WiFi.localIP().toString());
}

void setup_microphone() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_MIC_SCK,
    .ws_io_num = I2S_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SD
  };

  i2s_driver_install(I2S_MIC_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_MIC_PORT, &pin_config);
  Serial.println("Microfone INMP441 inicializado com sucesso.");
}

void record_audio() {
  Serial.println(">>> INICIANDO GRAVAÇÃO...");
  size_t bytes_read;
  i2s_read(I2S_MIC_PORT, audio_buffer, RECORD_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
  Serial.println(">>> GRAVAÇÃO CONCLUÍDA.");
}

void send_audio_to_api() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Erro: Wi-Fi desconectado.");
    return;
  }

  HTTPClient http;
  http.begin(api_url);
  
  // Como estamos enviando áudio RAW bruto e no backend definimos webm, 
  // numa versão madura converteríamos para WAV aqui no ESP32.
  // Por ora, enviamos como form-data genérico para a API.
  String boundary = "----ColarInteligenteBoundary";
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
  
  String head = "--" + boundary + "\r\n" +
                "Content-Disposition: form-data; name=\"audio\"; filename=\"audio.raw\"\r\n" +
                "Content-Type: application/octet-stream\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";

  size_t total_length = head.length() + RECORD_BUFFER_SIZE + tail.length();
  
  // (Pseudo-código para enviar em chunks devido à memória, 
  // ou envia direto se couber na PSRAM do XIAO ESP32-S3)
  
  // No ESP32-S3 com PSRAM, podemos fazer um request HTTP limpo alocando a payload inteira, 
  // mas aqui simplificamos a lógica para a fase de prova de conceito.
  
  Serial.println("Enviando áudio para o backend...");
  int httpResponseCode = http.POST((uint8_t*)audio_buffer, RECORD_BUFFER_SIZE); // Versão simplificada.
  
  if (httpResponseCode > 0) {
    Serial.print("Resposta do Backend (HTTP ");
    Serial.print(httpResponseCode);
    Serial.println(")");
    
    // Ler cabeçalhos (nossa API devolve os textos lá)
    // Coletar e tocar o áudio de resposta que chega via HTTP Stream (próxima etapa)
  } else {
    Serial.print("Falha na requisição HTTP. Erro: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  // O XIAO ESP32-S3 Sense possui 8MB de PSRAM. 
  // Alocar o buffer de gravação nela permite gravar áudios mais longos.
  audio_buffer = (uint8_t*) ps_malloc(RECORD_BUFFER_SIZE);
  if (audio_buffer == nullptr) {
    Serial.println("Falha ao alocar PSRAM para o áudio!");
    while(1); // Trava o sistema
  }

  setup_wifi();
  setup_microphone();
}

void loop() {
  // Lógica de simulação:
  // Na versão final, isso será ativado por um toque no acelerômetro ou botão touch.
  // Aqui, ele grava 5s, envia, e espera 15s antes de fazer de novo.
  
  record_audio();
  send_audio_to_api();
  
  Serial.println("Aguardando 15s para a próxima simulação...");
  delay(15000);
}

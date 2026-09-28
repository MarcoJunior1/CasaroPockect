# Casaro - Assistente de IA Vestível 🤖📿

O **Casaro** é um assistente de Inteligência Artificial vestível em formato de colar (estilo smartwatch inteligente). Ele utiliza a poderosa combinação de um microcontrolador ESP32-S3 com modelos de IA de ponta processados na nuvem (Groq Whisper para transcrição e LLMs rápidos como Qwen/Llama para conversação).

O dispositivo conta com uma tela LCD circular responsiva feita em LVGL, capta áudio via microfone I2S e devolve respostas sintetizadas em áudio e texto na tela.

---

## 🏗️ Arquitetura do Projeto

O projeto é dividido em três grandes módulos principais:

### 1. `backend/` (Servidor e IA)
Uma API construída em **Python e FastAPI** que serve como a "mente" do colar.
- Recebe o áudio enviado pelo dispositivo.
- Utiliza **Groq Whisper** para transcrição de fala para texto.
- Consulta um modelo de linguagem (LLM) rápido da **Groq (Qwen/Llama)**.
- Sintetiza a resposta de volta em áudio utilizando **gTTS**.

### 2. `frontend/` (Simulador de Comunicação)
Uma interface Web leve em HTML/JS projetada para testar a gravação e comunicação com o Backend diretamente do seu computador, antes de embarcar o código na placa física.

### 3. `wokwi_sim/` & `firmware/` (Firmware e UI do Dispositivo)
Código em **C++ (PlatformIO / Arduino)** que roda no **Seeed Studio XIAO ESP32-S3 Sense**.
- **Interface Gráfica:** Utiliza a biblioteca **LVGL** para renderizar a interface em uma tela circular GC9A01 (inclui avatar animado, status de bateria/wifi e texto deslizante).
- **Áudio & Conexão:** Gerencia a comunicação via protocolo I2S para o microfone INMP441 e alto-falante MAX98357A, e orquestra chamadas HTTP POST para o backend via Wi-Fi.
- O projeto `wokwi_sim/` contém a configuração pronta para ser emulada na nuvem ou no VS Code pelo simulador de hardwares **Wokwi**.

---

## 🚀 Como Rodar o Projeto

### Pré-requisitos
- Python 3.10+
- Visual Studio Code com as extensões **PlatformIO** e **Wokwi Simulator**
- Chave de API da [Groq](https://console.groq.com/)

### 1. Inicializando o Backend
Navegue até a pasta `backend`, instale as dependências e inicie o servidor:
```bash
cd backend
python -m venv venv
# No windows: .\venv\Scripts\activate
pip install -r requirements.txt

# Iniciar o servidor
python main.py
```
A API estará rodando em `http://localhost:8000`.

### 2. Simulando o Hardware (ESP32 + LVGL) no Wokwi
1. Abra a pasta `wokwi_sim` no VS Code.
2. Aguarde a extensão do **PlatformIO** configurar o projeto e baixar as bibliotecas gráficas (LVGL e TFT_eSPI).
3. Clique no botão de **Build** do PlatformIO (ícone de "✓" na barra inferior do VS Code) para compilar o `firmware.elf`.
4. Abra o arquivo `diagram.json` e pressione `F1` > **Wokwi: Start Simulator**.

---

## 🛠️ Hardware Físico Recomendado
- Microcontrolador: **Seeed Studio XIAO ESP32-S3 Sense**
- Display: **Tela LCD circular 1,28" (GC9A01 + Touch CST816S)**
- Microfone: **Módulo I2S INMP441**
- Alto-falante: **Amplificador MAX98357A + Mini Speaker 8Ω**
- Bateria: LiPo 3.7V (300mAh)

---

## 📝 Licença
Desenvolvido por **Marco Júnior** Livre para modificações e melhorias. 🚀


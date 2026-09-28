import os
import tempfile
from fastapi import FastAPI, UploadFile, File, Form, Response
from fastapi.responses import FileResponse
from fastapi.middleware.cors import CORSMiddleware
from groq import Groq
from gtts import gTTS
import urllib.parse

app = FastAPI()

# Configuração de CORS para permitir requisições do frontend local
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

GROQ_API_KEY = os.environ.get("GROQ_API_KEY", "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")
client = Groq(api_key=GROQ_API_KEY)

@app.post("/chat")
async def chat_with_ai(audio: UploadFile = File(None), text_input: str = Form(None)):
    # RF19: Se veio texto direto (ex: do Wokwi Serial), usa sem transcrever
    if text_input and text_input.strip():
        user_text = text_input.strip()
        print(f"[RF19] Texto direto recebido: {user_text}")
    elif audio and audio.filename:
        # Fluxo normal: salvar áudio e transcrever
        with tempfile.NamedTemporaryFile(delete=False, suffix=".webm") as temp_audio:
            temp_audio.write(await audio.read())
            temp_audio_path = temp_audio.name
        try:
            with open(temp_audio_path, "rb") as file:
                transcription = client.audio.transcriptions.create(
                    file=(temp_audio.name, file.read()),
                    model="whisper-large-v3",
                    language="pt",
                )
            user_text = transcription.text
            print(f"[Whisper] Transcrito: {user_text}")
        finally:
            if os.path.exists(temp_audio_path):
                os.remove(temp_audio_path)
    else:
        # Fallback: áudio fake do simulador
        user_text = "Olá, tudo bem com você?"
        print(f"[Fallback] Áudio fake detectado, usando pergunta padrão: {user_text}")

    # 3. Gerar a resposta com o LLM (Groq)
    chat_completion = client.chat.completions.create(
        messages=[
            {
                "role": "system",
                "content": "Você é um assistente de inteligência artificial acoplado em um colar vestível inteligente chamado Casaro. Suas respostas devem ser curtas, diretas e úteis, em português, pois serão reproduzidas em áudio."
            },
            {
                "role": "user",
                "content": user_text
            }
        ],
        model="qwen/qwen3.8-27b",
        temperature=0.5,
        max_tokens=256,
    )
    ai_response_text = chat_completion.choices[0].message.content
    print(f"[IA] Resposta: {ai_response_text}")

    # 4. Gerar o áudio da resposta usando gTTS
    tts = gTTS(text=ai_response_text, lang='pt')
    temp_response_audio = tempfile.NamedTemporaryFile(delete=False, suffix=".mp3")
    tts.save(temp_response_audio.name)

    # Codificar textos para enviar no header com segurança
    safe_user_text = urllib.parse.quote(user_text)
    safe_ai_response = urllib.parse.quote(ai_response_text)

    # 5. Retornar o arquivo de áudio
    return FileResponse(
        temp_response_audio.name,
        media_type="audio/mpeg",
        headers={
            "X-Transcribed-Text": safe_user_text,
            "X-AI-Response": safe_ai_response,
            "Access-Control-Expose-Headers": "X-Transcribed-Text, X-AI-Response"
        }
    )

@app.get("/")
def read_root():
    return {"message": "API do Colar Inteligente está rodando!"}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)

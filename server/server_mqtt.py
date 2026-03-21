import paho.mqtt.client as mqtt
import sqlite3
import json

# Configuração do Banco de Dados SQLite
DB_NAME = "sensores_iiot.db"

def init_db():
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    # Cria a tabela se ela não existir
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS leituras (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            topico TEXT,
            temperatura REAL,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    conn.commit()
    conn.close()

# Callback: O que fazer quando conectar no Broker
def on_connect(client, userdata, flags, rc, properties):
    print(f"📡 Conectado ao Broker MQTT com código: {rc}")
    # Assina o tópico
    client.subscribe("/tcc/cluster1/temperatura")
    print("🎧 Escutando o tópico: /tcc/cluster1/temperatura...")

# Callback: O que fazer quando chegar uma mensagem do ESP32
def on_message(client, userdata, msg):
    payload = msg.payload.decode('utf-8')
    print(f"\n📨 Nova mensagem no tópico {msg.topic}: {payload}")
    
    try:
        # Pelo seu log, o ESP manda uma lista com um dicionário: [{"temperatura": 21}]
        dados = json.loads(payload)
        
        # Puxa o valor da temperatura
        if isinstance(dados, list) and len(dados) > 0:
            temp = dados[0].get("temperatura")
        else:
            temp = dados.get("temperatura")
        
        # Salva no banco de dados
        if temp is not None:
            conn = sqlite3.connect(DB_NAME)
            cursor = conn.cursor()
            cursor.execute("INSERT INTO leituras (topico, temperatura) VALUES (?, ?)", (msg.topic, temp))
            conn.commit()
            conn.close()
            print(f"✅ Temperatura de {temp}°C gravada com sucesso no SQLite!")
            
    except json.JSONDecodeError:
        print("❌ Erro: O formato da mensagem não é um JSON válido.")
    except Exception as e:
        print(f"❌ Erro interno: {e}")

# Prepara o banco de dados
init_db()

# Prepara o Cliente MQTT e amarra as funções
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message

# Conecta no seu próprio computador (localhost)
client.connect("localhost", 1883, 60)

# Fica rodando eternamente esperando dados
client.loop_forever()
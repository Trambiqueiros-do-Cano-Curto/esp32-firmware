import paho.mqtt.client as mqtt
import sqlite3
import json

DB_NAME = "sensores_iiot.db"

def init_db():
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
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

def on_connect(client, userdata, flags, rc, properties):
    print(f"Conexão estabelecida com o Broker MQTT. Codigo de retorno: {rc}")
    client.subscribe("/tcc/cluster1/temperatura")
    print("Inscricao realizada no topico: /tcc/cluster1/temperatura")

def on_message(client, userdata, msg):
    payload = msg.payload.decode('utf-8')
    print(f"\nNova mensagem recebida - Topico: {msg.topic} | Payload: {payload}")
    
    try:
        dados = json.loads(payload)
        
        # Suporta payload no formato de lista de dicionarios ou dicionario unico
        if isinstance(dados, list) and len(dados) > 0:
            temp = dados[0].get("temperatura")
        else:
            temp = dados.get("temperatura")
        
        if temp is not None:
            conn = sqlite3.connect(DB_NAME)
            cursor = conn.cursor()
            cursor.execute("INSERT INTO leituras (topico, temperatura) VALUES (?, ?)", (msg.topic, temp))
            conn.commit()
            conn.close()
            print(f"Registro gravado: {temp}°C")
            
    except json.JSONDecodeError:
        print("Erro: Falha na decodificacao do payload JSON.")
    except Exception as e:
        print(f"Erro interno no processamento da mensagem: {e}")


init_db()

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)
client.loop_forever()
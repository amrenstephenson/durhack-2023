#https://pypi.org/project/websocket_client/
import websocket
import json
from confluent_kafka import Producer
import socket

API_TOKEN = json.load(open("secret.json", "r"))["finnhubToken"]

conf = {'bootstrap.servers': 'localhost:9092, localhost:9093, localhost:9094',
        'client.id': socket.gethostname()}

producer = Producer(conf)

def subscribe_to_symbol(web_socket, symbol):
    web_socket.send('{"type":"subscribe","symbol":"' + symbol + '"}')

def on_message(web_socket, message):
    json_data = json.loads(message)
    for datum in json_data["data"]:
        symbol = datum["s"]
        print(symbol)
        # del datum["s"]
        del datum["c"] # Delete trade conditions
        print(json.dumps(datum))
        producer.produce("symbols", key=symbol, value=json.dumps(datum), callback=acked)
    producer.flush()
        
def acked(err, msg):
    if err is not None:
        print("Failed to deliver message: %s: %s" % (str(msg), str(err)))
    # else:
    #     print("Message produced: %s" % (str(msg)))





def on_error(web_socket, error):
    print(error)

def on_close(web_socket):
    print("### closed ###")

def on_open(web_socket):
    for symbol in ["BINANCE:BTCUSDT", "BINANCE:LTCUSDT", "BINANCE:ETHUSDT", "BINANCE:BNBUSDT", "BINANCE:QTUMUSDT"]:
        subscribe_to_symbol(web_socket, symbol)

if __name__ == "__main__":
    # websocket.enableTrace(True)
    web_socket = websocket.WebSocketApp(f"wss://ws.finnhub.io?token={API_TOKEN}",
                              on_message = on_message,
                              on_error = on_error,
                              on_close = on_close)
    web_socket.on_open = on_open
    web_socket.run_forever()
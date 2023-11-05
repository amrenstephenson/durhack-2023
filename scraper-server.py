#https://pypi.org/project/websocket_client/
import websocket
import json

API_TOKEN = json.load(open("secret.json", "r"))["finnhubToken"]

def subscribe_to_symbol(web_socket, symbol):
    web_socket.send('{"type":"subscribe","symbol":"' + symbol + '"}')

def on_message(web_socket, message):
    print(json.dumps(json.loads(message), indent=2))

def on_error(web_socket, error):
    print(error)

def on_close(web_socket):
    print("### closed ###")

def on_open(web_socket):
    for symbol in ["BINANCE:BTCUSDT", "BINANCE:LTCUSDT", "BINANCE:ETHUSDT", "BINANCE:BNBUSDT", "BINANCE:QTUMUSDT"]:
        subscribe_to_symbol(web_socket, symbol)

if __name__ == "__main__":
    websocket.enableTrace(True)
    web_socket = websocket.WebSocketApp(f"wss://ws.finnhub.io?token={API_TOKEN}",
                              on_message = on_message,
                              on_error = on_error,
                              on_close = on_close)
    web_socket.on_open = on_open
    web_socket.run_forever()
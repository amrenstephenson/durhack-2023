from flask import Flask
from confluent_kafka import Consumer
import socket
import asyncio

conf = {'bootstrap.servers': 'localhost:9092, localhost:9093, localhost:9094',
        'group.id': 'singular-consumer-group'}
consumer = Consumer(conf)

consumer.subscribe(["symbols"])#, on_assign=reset_offsets)

app = Flask(__name__)

@app.route("/api/bubbles")
def hello_world():
    
    return "0.1"

def start_server():
    app.run(host="0.0.0.0", port=8080)

async def consume():
    while True:
        try:
            msg = consumer.poll()
            print("a")
            print(msg.value().decode())
        except RuntimeError as e:
            print("Consumer is closed.")
            break

loop = asyncio.get_event_loop()
loop.run_until_complete(consume())
loop.close()


if __name__ == "__main__":
    start_server()

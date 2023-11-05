from quart import Quart
from confluent_kafka import Consumer
import asyncio
import json
from dataclasses import dataclass
from math import log10
from concurrent.futures import ProcessPoolExecutor
import time

@dataclass
class Message():
    symbol: str
    unix_time: float
    last_price: float
    volume: float

conf = {'bootstrap.servers': 'localhost:9092, localhost:9093, localhost:9094',
        'group.id': 'singular-consumer-group'}
consumer = Consumer(conf)

consumer.subscribe(["symbols"])#, on_assign=reset_offsets)



app = Quart(__name__)

messages = {}

latest_times = {}

average_volumes = {}

@app.route("/")
def hello_world():
    return "Hello world!"

@app.route("/api/bubbles")
def bubbles():
    current_symbol = "BINANCE:ETHUSDT"
    if not current_symbol in average_volumes:
        return "-1"
    result = str(5 - 1.55 * log10(average_volumes[current_symbol])) + "," + current_symbol.split(":")[1]
    return result

@asyncio.coroutine
def run_server():
    app.run(host="0.0.0.0", port=8080)


async def consume():
    global messages
    while True:
        try:
            message_data = consumer.poll()
            json_data = json.loads(message_data.value().decode())
            symbol = json_data["s"]
            if not symbol in messages:
                messages[symbol] = []
            message = Message(
                symbol,
                unix_time=json_data["t"],
                last_price=json_data["p"],
                volume=json_data["v"],
            )
            messages[symbol].append(message)
            latest_times[symbol] = max(latest_times[symbol] if symbol in latest_times else 0, message.unix_time)
            if "BINANCE:ETHUSDT" in messages:
                print(len(messages["BINANCE:ETHUSDT"]))
            for symbol in messages:
                messages[symbol] = list(filter(lambda m: m.unix_time >= latest_times[symbol] - 10 * 1000, messages[symbol]))
                average_volumes[symbol] = sum([m.volume for m in messages[symbol]]) / len([symbol])
            # print(average_volumes)
            await asyncio.sleep(0)
        except RuntimeError as e:
            print("Consumer is closed.")
            break

@app.before_serving
async def startup():
    loop = asyncio.get_event_loop()
    loop.create_task(consume())

app.run(host="0.0.0.0", port=8080)
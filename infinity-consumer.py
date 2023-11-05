from quart import Quart
from confluent_kafka import Consumer
import asyncio
import json
from dataclasses import dataclass
from math import log10
from concurrent.futures import ProcessPoolExecutor
import time
import aioconsole
import logging

logging.getLogger('hypercorn.access').disabled = True

@dataclass
class Message():
    symbol: str
    unix_time: float
    last_price: float
    volume: float

SYMBOLS = json.load(open("settings.json", "r"))["symbols"]

conf = {'bootstrap.servers': 'localhost:29092, localhost:29093',
        'group.id': 'singular-consumer-group'}
consumer = Consumer(conf)
print(consumer)

consumer.subscribe(["symbols"])#, on_assign=reset_offsets)



app = Quart(__name__)

found_data = False

messages = {}

latest_times = {}

average_volumes = {}

current_symbol = "BINANCE:BTCUSDT"

@app.route("/")
def hello_world():
    return "Hello world!"

@app.route("/api/bubbles")
def bubbles():
    try:
        if not current_symbol in average_volumes:
            return "-1"
        result = str(max(0.75, min(5 - 1.55 * log10(average_volumes[current_symbol]), 5)))
        title = current_symbol.split(":")[-1]
        hue = (SYMBOLS.index(current_symbol) * 32) % 256
    except Exception as e:
        print(e)
    return ",".join([result, title, str(hue)])


async def run_server():
    app.run(host="0.0.0.0", port=8080)

async def consume():
    global messages, average_volumes, found_data
    while True:
        try:
            message_data = consumer.poll(timeout=1)
            if message_data is None:
                await asyncio.sleep(0.1)
                continue
            if not found_data:
                found_data = True
                print("Found data!")
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
            # if "BINANCE:ETHUSDT" in messages:
            #     print(len(messages["BINANCE:ETHUSDT"]))
            for symbol in messages:
                messages[symbol] = list(filter(lambda m: m.unix_time >= latest_times[symbol] - 10 * 1000, messages[symbol]))
                average_volumes[symbol] = sum([m.volume for m in messages[symbol]]) / (len(messages[symbol]) + 0.1)
            # print(average_volumes)
            await asyncio.sleep(0)
        except RuntimeError as e:
            print("Consumer is closed.")
            break

async def check_for_input():
    global current_symbol
    while True:
        await aioconsole.ainput('')
        current_symbol = SYMBOLS[(SYMBOLS.index(current_symbol) + 1) % len(SYMBOLS)]
        print(f"Now viewing {current_symbol}")


@app.before_serving
async def startup():
    loop = asyncio.get_event_loop()
    loop.create_task(check_for_input())
    loop.create_task(consume())

app.run(host="0.0.0.0", port=8080,)
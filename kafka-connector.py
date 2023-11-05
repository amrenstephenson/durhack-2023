from confluent_kafka import Producer
from confluent_kafka.admin import AdminClient, NewTopic
import socket
import asyncio
import time

print(socket.gethostname())
conf = {'bootstrap.servers': 'localhost:9092, localhost:9093, localhost:9094',
        'client.id': socket.gethostname()}

producer = Producer(conf)

def acked(err, msg):
    print("A")
    if err is not None:
        print("Failed to deliver message: %s: %s" % (str(msg), str(err)))
    else:
        print("Message produced: %s" % (str(msg)))


producer.produce("symbols", key="key12", value="value34", callback=acked)

# setup_topics()
producer.flush()
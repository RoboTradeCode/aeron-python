from time import sleep
from aeron import Subscriber

subscriber = Subscriber(lambda message: print(message), 'aeron:ipc')

while True:
    subscriber.poll()
    sleep(1)

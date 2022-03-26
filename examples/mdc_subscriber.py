from time import sleep
from aeron import Subscriber

# По умолчанию канал равен 'aeron:udp?control-mode=manual'
subscriber = Subscriber(lambda message: print(message))
subscriber.add_destination('aeron:udp?endpoint=localhost:40457|control=localhost:40456')

while True:
    subscriber.poll()
    sleep(1)

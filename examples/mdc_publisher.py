from time import sleep
from aeron import Publisher

publisher = Publisher('aeron:udp?control=localhost:40456|control-mode=dynamic')

for i in range(10):
    publisher.offer(f'Hello, World {i}')
    sleep(1)

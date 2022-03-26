from time import sleep
from aeron import Publisher

publisher = Publisher('aeron:ipc')

for i in range(10):
    publisher.offer(f'{i}')
    sleep(1)

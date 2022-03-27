from time import sleep
from aeron import Publisher


def main() -> None:
    publisher = Publisher(
        'aeron:udp?control=localhost:40656|control-mode=dynamic',  # channel
        1001                                                       # stream_id
    )

    for i in range(10000000):
        result = publisher.offer(f'Hello World! {i}')
        sleep(1)


if __name__ == '__main__':
    main()

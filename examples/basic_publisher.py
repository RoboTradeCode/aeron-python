from time import sleep
from aeron import Publisher


def main() -> None:
    publisher = Publisher(
        'aeron:udp?endpoint=localhost:20121',  # channel
        1001                                   # stream_id
    )

    for i in range(10000000):
        result = publisher.offer(f'Hello World! {i}')
        sleep(1)


if __name__ == '__main__':
    main()

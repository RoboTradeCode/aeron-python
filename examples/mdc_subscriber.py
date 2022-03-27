from time import sleep
from aeron import Subscriber


def handler(message: str) -> None:
    print(f'<<{message}>>')


def main() -> None:
    subscriber = Subscriber(
        handler,                          # handler
        'aeron:udp?control-mode=manual',  # channel
        1001,                             # stream_id
        10                                # fragment_limit
    )
    subscriber.add_destination('aeron:udp?endpoint=localhost:40657|control=localhost:40656')

    while True:
        result = subscriber.poll()
        sleep(1)


if __name__ == '__main__':
    main()

from time import sleep
from aeron import Subscriber


def handler(message: str) -> None:
    print(f'<<{message}>>')


def main() -> None:
    subscriber = Subscriber(
        handler,                               # handler
        'aeron:udp?endpoint=localhost:20121',  # channel
        1001,                                  # stream_id
        10                                     # fragment_limit
    )

    while True:
        result = subscriber.poll()
        sleep(1)


if __name__ == '__main__':
    main()

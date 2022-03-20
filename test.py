from time import sleep
from aeron import Subscriber


def handler(message: str) -> None:
    print(message)


def main() -> None:
    subscriber = Subscriber(handler, 'aeron:ipc')

    while True:
        subscriber.poll()
        sleep(1)


if __name__ == '__main__':
    main()

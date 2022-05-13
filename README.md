<img src="https://user-images.githubusercontent.com/44947427/160296335-a12f6887-850e-4170-86bc-fb509beea189.svg" height="101" alt="Python">

# aeron-python

[![Build](https://github.com/RoboTradeCode/aeron-python/actions/workflows/build.yml/badge.svg)](https://github.com/RoboTradeCode/aeron-python/actions/workflows/build.yml)
[![Python](https://img.shields.io/badge/python-3.8%20%7C%203.9%20%7C%203.10-blue)](https://www.python.org/downloads/)
[![CPython](https://img.shields.io/badge/implementation-cpython-blue)](https://github.com/python/cpython)
[![Linux](https://img.shields.io/badge/platform-linux-lightgrey)](https://ru.wikipedia.org/wiki/Linux)
[![Code style: black](https://img.shields.io/badge/code%20style-black-000000.svg)](https://github.com/psf/black)

Неофициальное расширение для Python, позволяющее использовать протокол [Aeron](https://github.com/real-logic/aeron).

## Установка

### Предварительные требования

Перед установкой и использованием данного расширения, у вас должен быть установлен Aeron. Вы можете воспользоваться
[статьёй в Wiki](https://github.com/RoboTradeCode/aeron-python/wiki/Установка-Aeron) для его установки.

### Сборка и установка расширения

```shell
pip install --upgrade "aeron @ git+ssh://git@github.com/RoboTradeCode/aeron-python.git"
```

> В примере выше используется подключение с помощью SSH. Подробнее о нём вы можете прочитать в
> руководстве ["Connecting to GitHub with SSH"](https://docs.github.com/en/authentication/connecting-to-github-with-ssh)

## Использование

### Отправка сообщений

```python
from aeron import Publisher

publisher = Publisher(
    channel="aeron:udp?endpoint=localhost:20121",  # str
    stream_id=1001,  # int
)

result = publisher.offer(message="Hello, World!")
publisher.close()
```

### Получение сообщений

```python
from aeron import Subscriber
from time import sleep


def handler(message: str) -> None:
    print(f"<<{message}>>")


subscriber = Subscriber(
    handler=handler,  # Callable[[str], None]
    channel="aeron:udp?endpoint=localhost:20121",  # str
    stream_id=1001,  # int
)

sleep(1)
fragments_read = subscriber.poll()
subscriber.close()
```

> Убедитесь, что у вас запущен медиа-драйвер Aeron перед использованием классов расширения

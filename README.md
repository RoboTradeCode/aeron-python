<img src="https://user-images.githubusercontent.com/44947427/160296335-a12f6887-850e-4170-86bc-fb509beea189.svg" height="101">

# aeron-python

[![Build](https://github.com/RoboTradeCode/aeron-python/actions/workflows/build.yml/badge.svg)](https://github.com/RoboTradeCode/aeron-python/actions/workflows/build.yml)
[![Python](https://img.shields.io/badge/python-3.8%20%7C%203.9%20%7C%203.10-blue)](https://www.python.org/downloads/)
[![CPython](https://img.shields.io/badge/implementation-cpython-blue)](https://github.com/python/cpython)
[![Linux](https://img.shields.io/badge/platform-linux-lightgrey)](https://ru.wikipedia.org/wiki/Linux)

Неофициальное расширение для Python, позволяющее использовать протокол [Aeron](https://github.com/real-logic/aeron)

## Установка

### Предварительные требования

Расширение является лишь обёрткой над клиентской реализацией протокола. В своей работе оно использует динамическую
библиотеку `libaeron.so`, которую будет искать в директории `/usr/local/lib`. А библиотека в свою очередь ожидает, что
на машине собран и запущен медиа-драйвер `aeronmd`.

Поэтому перед сборкой расширения, вы должны убедиться, что собрана сама библиотека и медиа-драйвер.

Если это ещё не сделано, то для сборки всего необходимого вы можете клонировать репозиторий и использовать
утилиту [CMake](https://cmake.org/):

```shell
mkdir -p build/Debug
cd build/Debug
cmake ../..
cmake --build . --target aeron aeronmd
sudo make install
cd ../..
```

> После сборки медиа-драйвер будет находиться в директории `build/Debug/libs/aeron/binaries`

#### Запуск медиа-драйвера

Вы можете просто запустить медиа-драйвер из терминала. Но более удобным способом является запуск в качестве
модуля [systemd](https://systemd.io/). Его конфигурация может выглядеть так:

```
[Unit]
Description=Aeron Media Driver
After=network.target

[Service]
User=ubuntu
ExecStart=/home/ubuntu/aeron-python/build/Debug/libs/aeron/binaries/aeronmd
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target
```

### Сборка пакета

Вы можете использовать [pip](https://pypi.org/project/pip/) для сборки и установки пакета:

```shell
pip install --upgrade git+https://${PERSONAL_ACCESS_TOKEN}@github.com/RoboTradeCode/aeron-python.git
```

> Репозиторий с исходным кодом является приватным. Для того чтобы клонировать его, в примере выше
> используется [персональный токен доступа](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/creating-a-personal-access-token),
> который сохранён в переменной окружения `PERSONAL_ACCESS_TOKEN`

## Использование

Расширение предоставляет классы `Subscriber` и `Publisher` для отправки и приёма сообщений соответственно.

### Subscriber

```python
from typing import Callable

class Subscriber:
    def __init__(self,
                 handler: Callable[[str], None],
                 channel: str = 'aeron:udp?control-mode=manual',
                 stream_id: int = 1001,
                 fragment_limit: int = 10):
        ...

    def add_destination(self, channel: str) -> int:
        ...

    def remove_destination(self, channel: str) -> int:
        ...

    def poll(self) -> int:
        ...
```

### Publisher

```python
class Publisher:
    def __init__(self, channel: str = 'aeron:udp?control-mode=manual', stream_id: int = 1001):
        ...

    def offer(self, message: str) -> int:
        ...
```

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

Расширение предоставляет 2 класса для отправки и приёма сообщений — [`Publisher`](aeronmodule.pyi)
и [`Subscriber`](aeronmodule.pyi) соответственно. Для того чтобы связать их, вам потребуется:

1. Канал
2. Идентификатор потока

**Канал (Channel)** задаётся строкой, которая
собирается [по определённым правилам](https://github.com/real-logic/aeron/wiki/Channel-Configuration). В начале строки
находится медиа-значение — оно определяет протокол, который Aeron будет использовать для отправки
сообщений ([IPC](https://github.com/real-logic/aeron/wiki/Channel-Configuration#ipc-media---inter-process-communication)
или [UDP](https://github.com/real-logic/aeron/wiki/Channel-Configuration#udp-media)). После медиа-значения идут
параметры в формате `ключ=значение`.

Например, канал для простой однонаправленной передачи данных по UDP будет выглядеть так:

```
aeron:udp?endpoint=localhost:20121
```

**Идентификатор потока (Stream ID)** задаётся числом (любым, кроме 0). Он нужен, чтобы отличать потоки данных внутри
одного канала, так как в Aeron по одному и тому же каналу можно передавать несколько независимых потоков данных.

> По умолчанию идентификатор потока равняется `1001`. Такое же значение
> используют [примеры в репозитории Aeron](https://github.com/real-logic/aeron/tree/master/aeron-samples/src/main/c).

### Publisher

```python
from aeron import Publisher

publisher = Publisher('aeron:udp?endpoint=localhost:20121', 1001)
publisher.offer('Hello World!')
```

Publisher принимает 2 позиционных аргумента — канал и идентификатор потока. Для отправки сообщения используется
метод `offer`, который единственным позиционным аргументом принимает строку.

### Subscriber

```python
from aeron import Subscriber


def handler(message: str) -> None:
    pass


subscriber = Subscriber(handler, 'aeron:udp?endpoint=localhost:20121', 1001, 10)
subscriber.poll()
```

Subscriber принимает 4 позиционных аргумента — функцию обратного вызова, канал, идентификатор потока и максимальное
количество фрагментов в сообщении.

Под фрагментом сообщения подразумевается фрагмент соответствующего IP-пакета. Сообщение может быть фрагментировано в
случае, если оно превышает [MTU](https://ru.wikipedia.org/wiki/Maximum_transmission_unit). Aeron автоматически соберёт
сообщение из фрагментов и только затем передаст в функцию обратного вызова. Но если количество фрагментов превысит
максимальное значение — сообщение будет полностью отброшено.

> Средний размер MTU в сети Интернет составляет 1400 байт. Следовательно, если максимально количество фрагментов равно
> 10, вы сможете передать по каналу Aeron сообщение примерно равное 14000 байт

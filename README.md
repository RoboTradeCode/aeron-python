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

Перед установкой и использованием данного расширения, у вас должен быть установлен сам Aeron. Если это не так, то
установите его, воспользовавшись [официальным руководством](https://github.com/real-logic/aeron#c-build):

1. Установите зависимости сборки:

```shell
sudo apt update
sudo apt install --assume-yes git cmake g++ default-jdk libbsd-dev uuid-dev
git clone --branch 1.38.2 --depth 1 https://github.com/real-logic/aeron.git
```

2. Соберите и протестируйте код:

```shell
cd aeron
mkdir --parents cppbuild/Debug
cd cppbuild/Debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
cmake --build . --clean-first
ctest
```

> Вы можете ускорить сборку, указав команде `cmake --build` максимальное количество параллельных процессов. За это
> отвечает параметр [`--parallel`](https://cmake.org/cmake/help/latest/manual/cmake.1.html#build-a-project)

3. Установите библиотеку в систему

```shell
sudo cmake --install .
```

> По умолчанию CMake установит библиотеку в `/usr/local`. Вы можете изменить директорию установки с помощью
> параметра [`--prefix`](https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html#variable:CMAKE_INSTALL_PREFIX)

### Сборка и установка расширения

```shell
pip install --upgrade "aeron @ git+ssh://git@github.com/RoboTradeCode/aeron-python.git@v0.2.0"
```

> В примере выше используется подключение с помощью SSH. Подробнее о нём вы можете прочитать в
> руководстве ["Connecting to GitHub with SSH"](https://docs.github.com/en/authentication/connecting-to-github-with-ssh)

## Использование

Расширение является клиентской частью протокола Aeron. Это означает, что для его использования вам также потребуется
запустить медиа-драйвер.

Расширение предоставляет 2 класса для отправки и приёма сообщений — [`Publisher`](src/aeron/aeronmodule.pyi)
и [`Subscriber`](src/aeron/aeronmodule.pyi) соответственно. Для того чтобы связать их, вам потребуется:

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


subscriber = Subscriber(handler,
                        channel='aeron:udp?endpoint=localhost:20121',
                        stream_id=1001,
                        fragment_limit=10)
subscriber.poll()
```

Subscriber принимает 4 позиционных аргумента — функцию обратного вызова, канал, идентификатор потока и максимальное
количество фрагментов в сообщении. Для получения сообщений используется метод `poll`.

Под фрагментом сообщения подразумевается фрагмент соответствующего IP-пакета. Сообщение может быть фрагментировано в
случае, если оно превышает [MTU](https://ru.wikipedia.org/wiki/Maximum_transmission_unit). Aeron автоматически соберёт
сообщение из фрагментов и только затем передаст в функцию обратного вызова. Но если количество фрагментов превысит
максимальное значение — сообщение будет полностью отброшено.

> Средний размер MTU в сети Интернет составляет 1400 байт. Следовательно, если максимально количество фрагментов равно
> 10, вы сможете передать по каналу Aeron сообщение примерно равное 14000 байт

### Multicast

```python
publisher = Publisher('aeron:udp?control=localhost:40456|control-mode=dynamic', 1001)

subscriber = Subscriber(handler, 'aeron:udp?control-mode=manual', 1001, 10)
subscriber.add_destination('aeron:udp?endpoint=localhost:40457|control=localhost:40456')
```

Если требуется передавать
данные [нескольким получателям](https://github.com/real-logic/aeron/wiki/Multiple-Destinations) одновременно, нужно
указать параметры канала `control` и `control-mode`.

Параметр `control` задаёт адрес, с которого осуществляется рассылка, а параметр `control-mode` задаёт режим управления.

Существует два режима управления: ручной (`manual`) и динамический (`dynamic`). Для Publisher'а расширение предлагает
использовать только динамический режим, в нём приём новых соединений происходит автоматически. Для Subscriber'а вы
можете использовать как динамический, так и ручной режим.

Ручной режим для подписчика позволяет программно подписаться на любое количество рассылок. Для этого используется метод
`add_destination`. Метод принимает один позиционный аргумент — канал, на который требуется подписаться.

#### Реальный пример

![Multicast](https://user-images.githubusercontent.com/44947427/160488339-fb5ed869-43de-4aa6-8fa6-325c923b3419.svg)

```python
# 3.65.14.242 (172.31.14.205)

publisher = Publisher('aeron:udp?control=172.31.14.205:40456|control-mode=dynamic')

subscriber = Subscriber(handler, 'aeron:udp?control-mode=manual')
subscriber.add_destination('aeron:udp?endpoint=172.31.14.205:40457|control=172.31.14.205:40456')
subscriber.add_destination('aeron:udp?endpoint=172.31.14.205:40458|control=18.159.92.185:40456')
```

```python
# 18.159.92.185 (172.31.4.173)

publisher = Publisher('aeron:udp?control=172.31.4.173:40456|control-mode=dynamic')

subscriber = Subscriber(handler, 'aeron:udp?control-mode=manual')
subscriber.add_destination('aeron:udp?endpoint=172.31.4.173:40457|control=172.31.4.173:40456')
subscriber.add_destination('aeron:udp?endpoint=172.31.4.173:40458|control=3.65.14.242:40456')
```

# aeron-python

Неофициальное расширение для Python, позволяющее использовать протокол [Aeron](https://github.com/real-logic/aeron)

# Установка

Репозиторий с исходным кодом является приватным. Поэтому для его клонирования вам может
потребоваться [персональный токен доступа](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/creating-a-personal-access-token).
Если вы сохраните его в переменной окружения `PERSONAL_ACCESS_TOKEN`, то сможете воспользоваться следующей командой для
сборки и установки пакета:

```shell
pip install -U git+https://${PERSONAL_ACCESS_TOKEN}@github.com/RoboTradeCode/aeron-python.git
```

# Использование

```python
from aeron import Subscriber, Publisher

subscriber = Subscriber(lambda message: print(message), 'aeron:ipc')
publisher = Publisher('aeron:ipc')

for i in range(10):
    publisher.offer(str(i))
    subscriber.poll()
```

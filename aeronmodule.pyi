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


class Publisher:
    def __init__(self, channel: str = 'aeron:udp?control-mode=manual', stream_id: int = 1001):
        ...

    def offer(self, message: str) -> int:
        ...

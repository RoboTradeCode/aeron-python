from typing import Callable

class Publisher:
    def __init__(
        self,
        channel: str = "aeron:udp?endpoint=localhost:20121",
        stream_id: int = 1001,
    ): ...
    def offer(self, message: str) -> int: ...
    def close(self) -> None: ...

class Subscriber:
    def __init__(
        self,
        handler: Callable[[str], None],
        channel: str = "aeron:udp?endpoint=localhost:20121",
        stream_id: int = 1001,
    ): ...
    def poll(self) -> int: ...
    def close(self) -> None: ...

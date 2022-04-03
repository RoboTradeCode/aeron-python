from typing import Any, Callable, Optional

class Subscriber:
    def __init__(
        self,
        handler: Callable[[Optional[Any], str], None],
        channel: str = "aeron:udp?control-mode=manual",
        stream_id: int = 1001,
        fragment_limit: int = 10,
        clientd: Optional[Any] = None,
    ): ...
    def add_destination(self, channel: str) -> int: ...
    def remove_destination(self, channel: str) -> int: ...
    def poll(self) -> int: ...

class Publisher:
    def __init__(
        self, channel: str = "aeron:udp?control-mode=manual", stream_id: int = 1001
    ): ...
    def offer(self, message: str) -> int: ...

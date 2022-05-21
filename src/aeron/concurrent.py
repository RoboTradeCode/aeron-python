import asyncio
import time
from abc import ABC, abstractmethod


class IdleStrategy(ABC):
    @abstractmethod
    def idle(self, work_count: int = None) -> None:
        ...


class SleepingIdleStrategy(IdleStrategy):
    def __init__(self, duration: int):
        self._m_duration = duration

    def idle(self, work_count: int = None) -> None:
        if work_count is None or work_count == 0:
            time.sleep(self._m_duration / 1000)


class YieldingIdleStrategy(IdleStrategy):
    def idle(self, work_count: int = None) -> None:
        if work_count is None or work_count == 0:
            time.sleep(0)


class NoOpIdleStrategy(IdleStrategy):
    def idle(self, work_count: int = None) -> None:
        pass


class AsyncIdleStrategy(ABC):
    @abstractmethod
    async def idle(self, work_count: int = None) -> None:
        ...


class AsyncSleepingIdleStrategy(AsyncIdleStrategy):
    def __init__(self, duration: int):
        self._m_duration = duration

    async def idle(self, work_count: int = None) -> None:
        if work_count is None or work_count == 0:
            await asyncio.sleep(self._m_duration / 1000)


class AsyncYieldingIdleStrategy(AsyncIdleStrategy):
    async def idle(self, work_count: int = None) -> None:
        if work_count is None or work_count == 0:
            await asyncio.sleep(0)


class AsyncNoOpIdleStrategy(AsyncIdleStrategy):
    async def idle(self, work_count: int = None) -> None:
        pass

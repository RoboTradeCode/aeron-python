from abc import ABC, abstractmethod
from time import sleep


class IdleStrategy(ABC):
    @abstractmethod
    def idle(self, work_count: int = None) -> None:
        ...


class SleepingIdleStrategy(IdleStrategy):
    def __init__(self, duration: int):
        self._m_duration = duration

    def idle(self, work_count: int = None) -> None:
        if work_count is None or work_count == 0:
            sleep(self._m_duration)


class YieldingIdleStrategy(IdleStrategy):
    # noinspection PyMethodMayBeStatic
    def idle(self, work_count: int = None) -> None:
        if work_count is None or work_count == 0:
            sleep(0)


class NoOpIdleStrategy(IdleStrategy):
    def idle(self, work_count: int = None) -> None:
        pass

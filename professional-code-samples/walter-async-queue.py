"""
Asynchronous queue pattern for decoupled producer-consumer architecture.
Enables independent execution of data acquisition, analysis, and execution
without blocking on any single component.
"""

import asyncio
from dataclasses import dataclass
from typing import Optional
from datetime import datetime


@dataclass
class MarketData:
    timestamp: datetime
    price: float
    volume: int
    source: str


@dataclass
class Signal:
    timestamp: datetime
    signal_type: str
    confidence: float
    metadata: dict


class AsyncDataPipeline:
    """
    Decoupled async pipeline with independent producer and consumer components.
    Queue-based communication prevents blocking between stages.
    """

    def __init__(self, queue_size: int = 1000):
        self.data_queue: asyncio.Queue = asyncio.Queue(maxsize=queue_size)
        self.signal_queue: asyncio.Queue = asyncio.Queue(maxsize=queue_size)

    async def data_producer(self, source_coroutine) -> None:
        """
        Continuously acquire market data from source.
        Enqueues data without waiting for consumer.
        """
        async for market_data in source_coroutine:
            try:
                self.data_queue.put_nowait(market_data)
            except asyncio.QueueFull:
                # Queue full: drop oldest data to prevent blocking
                try:
                    self.data_queue.get_nowait()
                    self.data_queue.put_nowait(market_data)
                except asyncio.QueueEmpty:
                    pass

    async def analysis_worker(self, analyzer_func) -> None:
        """
        Consume market data, perform analysis, produce signals.
        Executes independently of data production rate.
        """
        while True:
            try:
                market_data = await asyncio.wait_for(
                    self.data_queue.get(), timeout=5.0
                )
            except asyncio.TimeoutError:
                continue

            signal = await analyzer_func(market_data)
            if signal:
                try:
                    self.signal_queue.put_nowait(signal)
                except asyncio.QueueFull:
                    pass

    async def execution_consumer(self, executor_func) -> None:
        """
        Consume signals and execute actions.
        Processes signals at its own rate independent of analysis speed.
        """
        while True:
            try:
                signal = await asyncio.wait_for(
                    self.signal_queue.get(), timeout=10.0
                )
            except asyncio.TimeoutError:
                continue

            await executor_func(signal)

    async def run(
        self,
        data_source,
        analyzer_func,
        executor_func,
    ) -> None:
        """
        Execute all pipeline stages concurrently.
        Each component operates independently on its own schedule.
        """
        tasks = [
            asyncio.create_task(self.data_producer(data_source)),
            asyncio.create_task(self.analysis_worker(analyzer_func)),
            asyncio.create_task(self.execution_consumer(executor_func)),
        ]

        # Run until explicit cancellation
        await asyncio.gather(*tasks)

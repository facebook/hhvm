# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# pyre-strict

from typing import AsyncGenerator, Generic, TypeVar

from thrift.python.streaming.stream import ClientBufferedStream

TChunk = TypeVar("TChunk")
TFinalResponse = TypeVar("TFinalResponse")

class ClientSink(Generic[TChunk, TFinalResponse]):
    def __init__(self) -> None: ...
    async def sink(
        self, gen: AsyncGenerator[TChunk, None]
    ) -> Generic[TFinalResponse]: ...

TSinkChunk = TypeVar("TSinkChunk")
TStreamChunk = TypeVar("TStreamChunk")
TResponse = TypeVar("TResponse")

class BidirectionalStream(Generic[TSinkChunk, TStreamChunk]):
    sink: ClientSink[TSinkChunk, None]
    stream: ClientBufferedStream[TStreamChunk]
    def __init__(self) -> None: ...

class ResponseAndBidirectionalStream(Generic[TResponse, TSinkChunk, TStreamChunk]):
    response: TResponse
    sink: ClientSink[TSinkChunk, None]
    stream: ClientBufferedStream[TStreamChunk]
    def __init__(self) -> None: ...

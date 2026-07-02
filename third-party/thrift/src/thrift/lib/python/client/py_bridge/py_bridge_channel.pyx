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

# cython: c_string_type=unicode, c_string_encoding=utf8

"""Bridge that lets a generated thrift-python client run over a pure-Python
transport (a `ChannelHandler`) instead of a C++ socket channel.

`get_bridged_async_client` builds a `PyBridgeRequestChannel` (C++) whose sender
calls back into `bridge_send_to_python`, which runs the handler's `send_request`
coroutine and fulfills the C++ promise with the reply. The generated client's
typed methods, enveloping (via OmniClient), and reply/exception decoding are all
reused unchanged."""

import asyncio
import logging

from cpython.ref cimport PyObject
from folly cimport cFollyPromise
from folly.executor cimport get_executor
from folly.iobuf cimport cIOBuf, const_uchar, copyBuffer, from_unique_ptr, IOBuf
from libc.stdint cimport uint16_t, uint64_t
from libcpp.memory cimport unique_ptr
from libcpp.utility cimport move as cmove
from thrift.python.client.request_channel cimport RequestChannel
from thrift.python.exceptions cimport (
    cTApplicationException,
    cTApplicationExceptionType__UNKNOWN,
)
from thrift.python.protocol cimport Protocol as cProtocol
from thrift.python.streaming.py_promise cimport Promise_IOBuf


logger: logging.Logger = logging.getLogger(__name__)

# asyncio keeps only weak references to tasks, so an in-flight send could be
# garbage-collected (and silently cancelled) before it completes. Hold a strong
# reference until each task settles.
_inflight_tasks: set = set()


cdef IOBuf _bytes_to_iobuf(bytes data):
    return from_unique_ptr(
        copyBuffer(<const_uchar*><const char*>data, <uint64_t>len(data))
    )


cdef _error_promise(Promise_IOBuf promise, object ex):
    # Surface only the exception class name to the client; the full message/args
    # (potentially sensitive) go to the server log via logger.exception, not into
    # the cross-process error string.
    promise.error_ta(
        cTApplicationException(
            cTApplicationExceptionType__UNKNOWN, type(ex).__name__.encode("utf-8")
        )
    )


async def _run_handler(
    object handler, IOBuf request, int rpc_kind, Promise_IOBuf promise
):
    try:
        reply = await handler.send_request(bytes(request), rpc_kind)
        # Convert inside the try so a non-bytes reply is reported through the
        # promise rather than escaping as an uncompleted (broken) promise.
        iobuf = _bytes_to_iobuf(reply)
    except Exception as ex:
        logger.exception("glue bridge ChannelHandler.send_request failed")
        _error_promise(promise, ex)
        return
    except BaseException as ex:
        # CancelledError / SystemExit / KeyboardInterrupt / GeneratorExit are
        # BaseExceptions and would otherwise bypass the handler above, leaving
        # the promise uncompleted and hanging the C++ caller. Complete it, then
        # re-raise so cancellation / interpreter teardown still propagates. The
        # nested guard ensures the re-raise happens even if _error_promise fails.
        try:
            _error_promise(promise, ex)
        except BaseException:
            logger.exception("glue bridge failed to error promise on teardown")
        raise
    # Guard the success completion too: if complete() raises (e.g. the promise
    # was already satisfied), the exception would escape the task and leave the
    # contract ambiguous. Log instead; the future is already resolved or broken.
    try:
        promise.complete(iobuf)
    except BaseException:
        logger.exception("glue bridge failed to complete promise")


cdef api void bridge_send_to_python(
    object handler,
    unique_ptr[cIOBuf] request,
    int rpc_kind,
    cFollyPromise[unique_ptr[cIOBuf]] promise,
) noexcept:
    # noexcept: any uncaught raise here is swallowed by Cython and would leave
    # the C++ caller's future hanging forever, so every path must either complete
    # the promise or let the moved cFollyPromise break the future on destruction.
    cdef Promise_IOBuf p
    cdef IOBuf req
    try:
        # If create() itself fails, the moved cFollyPromise is destroyed, which
        # breaks the caller's future (an error, not a hang); we have no wrapper
        # left to report through, so just return.
        p = Promise_IOBuf.create(cmove(promise))
    except BaseException:
        logger.exception("glue bridge failed to wrap promise")
        return
    try:
        # get_running_loop() is correct here — this is always invoked from the
        # asyncio loop via get_executor().
        req = from_unique_ptr(cmove(request))
        task = asyncio.get_running_loop().create_task(
            _run_handler(handler, req, rpc_kind, p)
        )
        # Strong ref until the task settles, so it is not GC'd mid-flight.
        _inflight_tasks.add(task)
        task.add_done_callback(_inflight_tasks.discard)
    except BaseException as ex:
        logger.exception("glue bridge failed to schedule send")
        _error_promise(p, ex)


cdef RequestChannel _make_channel(object handler, uint16_t protocol_id):
    return RequestChannel.create(
        makeBridgedChannel(<PyObject*>handler, protocol_id, get_executor())
    )


def get_bridged_async_client(
    client_klass, handler, cProtocol protocol = cProtocol.COMPACT
):
    """Return an instance of `client_klass.Async` routed through `handler`.

    `handler` must expose `async def send_request(enveloped_request: bytes,
    rpc_kind: int) -> bytes`. Use the result as an async context manager.
    """
    async_klass = getattr(client_klass, "Async", None)
    if async_klass is None:
        raise TypeError(
            f"{client_klass!r} has no `.Async`; pass a generated thrift-python "
            "client class (e.g. MyService)"
        )
    # Require a callable `send_request` to fail fast on the common misuse (wrong
    # object / missing method) without rejecting valid indirect coroutine
    # functions (functools.partial, decorated, or instance __call__) that a
    # stricter iscoroutinefunction check would wrongly refuse.
    if not callable(getattr(handler, "send_request", None)):
        raise TypeError(
            "handler must define an async `send_request(enveloped_request: "
            "bytes, rpc_kind: int) -> bytes` method"
        )
    client = async_klass()
    client._set_channel(_make_channel(handler, <uint16_t>protocol))
    return client

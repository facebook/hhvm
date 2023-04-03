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

# pyre-unsafe

from __future__ import absolute_import, division, print_function, unicode_literals

import contextlib
import logging
import os
from concurrent.futures import Future
from functools import partial

import six

try:
    import resource
except ImportError:
    # Windows doesn't have this
    resource = None


from thrift.protocol import THeaderProtocol
from thrift.Thrift import (
    TApplicationException,
    TException,
    TMessageType,
    TRequestContext,
)


log = logging.getLogger(__name__)


def get_memory_usage():
    # this parses the resident set size from /proc/self/stat, which
    # is the same approach the C++ FacebookBase takes
    with open("/proc/self/stat") as stat:
        stat_string = stat.read()
    # rss is field number 23 in /proc/pid/stat, see man proc
    # for the full list
    rss_pages = int(stat_string.split()[23])
    # /proc/pid/stat excludes 3 administrative pages from the count
    rss_pages += 3
    return rss_pages * resource.getpagesize()


def get_function_name(func):
    return func.__name__.split("_", 1)[-1]


def make_unknown_function_exception(name):
    return TApplicationException(
        TApplicationException.UNKNOWN_METHOD,
        "Unknown function {!r}".format(name),
    )


def process_main(asyncio=False):
    """Decorator for process method."""
    if asyncio:
        from asyncio import Future

    def _decorator(func):
        def nested(self, iprot, oprot, server_ctx=None):
            # self is a TProcessor instance
            name, seqid = self.readMessageBegin(iprot)
            if self.doesKnowFunction(name):
                ret = self.callFunction(name, seqid, iprot, oprot, server_ctx)
                if asyncio:
                    return ret  # a Deferred/Future

                return True
            self.skipMessageStruct(iprot)
            exc = make_unknown_function_exception(name)
            self.writeException(oprot, name, seqid, exc)
            if asyncio:
                fut = Future(loop=self._loop)
                fut.set_result(None)
                return fut

        return nested

    return _decorator


try:
    # You can use the following for last resort memory leak debugging if Heapy
    # and other clean room methods fail. Setting this to multiple megabytes
    # will expose unexpectedly heavy requests.

    # Note that in case of multithreading use, there's sometimes going to be
    # multiple requests reporting a single bump in memory usage. Look at the
    # slowest request first (it will present the biggest bump).

    # Also note this is unsupported on Twisted/asyncio.
    MEMORY_WARNING_THRESHOLD = int(
        os.environ.get("THRIFT_PER_REQUEST_MEMORY_WARNING", 0)
    )
except (TypeError, ValueError):
    MEMORY_WARNING_THRESHOLD = 0

_process_method_mem_usage = get_memory_usage if MEMORY_WARNING_THRESHOLD else lambda: 0


def needs_request_context(processor):
    return hasattr(processor._handler, "setRequestContext")


def set_request_context(processor, iprot):
    if needs_request_context(processor):
        request_context = TRequestContext()
        if isinstance(iprot, THeaderProtocol.THeaderProtocol):
            request_context.setHeaders(iprot.trans.get_headers())
        processor._handler.setRequestContext(request_context)


def reset_request_context(processor):
    if needs_request_context(processor):
        processor._handler.setRequestContext(None)


def process_method(argtype, oneway=False, asyncio=False):
    """Decorator for process_xxx methods."""

    def _decorator(func):
        def nested(self, seqid, iprot, oprot, server_ctx):
            _mem_before = _process_method_mem_usage()
            fn_name = get_function_name(func)
            # self is a TProcessor instance
            handler_ctx = self._event_handler.getHandlerContext(
                fn_name,
                server_ctx,
            )
            set_request_context(self, iprot)
            try:
                args = self.readArgs(iprot, handler_ctx, fn_name, argtype)
            except Exception as e:
                args = argtype()
                log.exception(
                    "Exception thrown while reading arguments: `%s`",
                    str(e),
                )
                result = TApplicationException(message=str(e))
                if not oneway:
                    self.writeException(oprot, fn_name, seqid, result)
            else:
                if asyncio:
                    return func(self, args, handler_ctx, seqid, oprot, fn_name)

                result = func(self, args, handler_ctx)
                if not oneway:
                    self.writeReply(
                        oprot, handler_ctx, fn_name, seqid, result, server_ctx
                    )
            finally:
                reset_request_context(self)

            _mem_after = _process_method_mem_usage()
            if _mem_after - _mem_before > MEMORY_WARNING_THRESHOLD:
                log.error(
                    "Memory usage rose from %d to %d while processing `%s` "
                    "with args `%s`",
                    _mem_before,
                    _mem_after,
                    fn_name,
                    str(args),
                )

        return nested

    return _decorator


def future_process_main():
    """Decorator for process method of future processor."""

    def _decorator(func):
        def nested(self, iprot, oprot, server_ctx=None):
            name, seqid = self.readMessageBegin(iprot)
            if self.doesKnowFunction(name):
                return self._processMap[name](self, seqid, iprot, oprot, server_ctx)
            else:
                self.skipMessageStruct(iprot)
                exc = make_unknown_function_exception(name)
                self.writeException(oprot, name, seqid, exc)
                fut = Future(self._loop)
                fut.set_result(True)
                return fut

        return nested

    return _decorator


def write_result(result, reply_type, seqid, event_handler, handler_ctx, fn_name, oprot):

    event_handler.preWrite(handler_ctx, fn_name, result)

    try:
        oprot.writeMessageBegin(fn_name, reply_type, seqid)

        # Thrift serialization here could fail due to format error
        result.write(oprot)

        oprot.writeMessageEnd()
        oprot.trans.flush()

    except Exception as e:
        # Handle any thrift serialization exceptions

        # Transport is likely in a messed up state. Some data may already have
        # been written and it may not be possible to recover. Doing nothing
        # causes the client to wait until the request times out. Try to
        # close the connection to trigger a quicker failure on client side
        oprot.trans.close()

        # Let application know that there has been an exception
        event_handler.handlerError(handler_ctx, fn_name, e)

        # We raise the exception again to avoid any further processing
        raise

    finally:
        # Since we called preWrite, we should also call postWrite to
        # allow application to properly log their requests
        event_handler.postWrite(handler_ctx, fn_name, result)


def _done(future, processor, handler_ctx, fn_name, oprot, reply_type, seqid, oneway):
    try:
        result = future.result()
    except TApplicationException as e:
        if oneway:
            result = None
        else:
            result = e

    if result is None:
        return

    if isinstance(result, TApplicationException):
        reply_type = TMessageType.EXCEPTION

    write_result(
        result, reply_type, seqid, processor._event_handler, handler_ctx, fn_name, oprot
    )


def future_process_method(argtype, oneway=False):
    """Decorator for process_xxx methods of futuer processor.

    TODO haijunz: handler_ctx is not supported in FutureProcessor's handler
    methods. handler_ctx should only be used by processor event handlers
    and ContextProcessor should be deprecated. To pass things to handler
    methods, use request context.
    """

    def _decorator(func):
        def nested(self, seqid, iprot, oprot, server_ctx):
            fn_name = func.__name__.split("_", 1)[-1]
            handler_ctx = self._event_handler.getHandlerContext(fn_name, server_ctx)
            args = argtype()
            reply_type = TMessageType.REPLY
            self._event_handler.preRead(handler_ctx, fn_name, args)
            args.read(iprot)
            iprot.readMessageEnd()
            self._event_handler.postRead(handler_ctx, fn_name, args)

            if hasattr(self._handler, "setRequestContext"):
                request_context = TRequestContext()
                if isinstance(iprot, THeaderProtocol.THeaderProtocol):
                    request_context.setHeaders(iprot.trans.get_headers())
                self._handler.setRequestContext(request_context)

            fut = func(self, args, handler_ctx)
            done_callback = partial(
                _done,
                processor=self,
                handler_ctx=handler_ctx,
                fn_name=fn_name,
                oprot=oprot,
                reply_type=reply_type,
                seqid=seqid,
                oneway=oneway,
            )
            fut.add_done_callback(done_callback)

            if hasattr(self._handler, "setRequestContext"):
                self._handler.setRequestContext(None)
            return fut

        return nested

    return _decorator


def write_results_after_future(
    result,
    event_handler,
    handler_ctx,
    seqid,
    oprot,
    fn_name,
    known_exceptions,
    future,
):
    """Result/exception handler for asyncio futures."""
    try:
        try:
            result.success = future.result()
            reply_type = TMessageType.REPLY
        except TException as e:
            for exc_name, exc_type in known_exceptions.items():
                if isinstance(e, exc_type):
                    setattr(result, exc_name, e)
                    reply_type = TMessageType.REPLY
                    event_handler.handlerException(handler_ctx, fn_name, e)
                    break
            else:
                raise
    except Exception as e:
        result = TApplicationException(message=str(e))
        reply_type = TMessageType.EXCEPTION
        event_handler.handlerError(handler_ctx, fn_name, e)

    write_result(result, reply_type, seqid, event_handler, handler_ctx, fn_name, oprot)


@contextlib.contextmanager
def protocol_manager(protocol):
    try:
        yield protocol.client
    finally:
        protocol.close()


def run_on_thread(func):
    func._run_on_thread = True
    return func


def should_run_on_thread(func):
    return getattr(func, "_run_on_thread", False)

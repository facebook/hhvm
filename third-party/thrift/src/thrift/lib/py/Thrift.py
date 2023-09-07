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

from __future__ import absolute_import, division, print_function, unicode_literals

import logging
import sys
import threading

import six

UEXW_MAX_LENGTH = 1024


class TType:
    STOP = 0
    VOID = 1
    BOOL = 2
    BYTE = 3
    I08 = 3
    DOUBLE = 4
    I16 = 6
    I32 = 8
    I64 = 10
    STRING = 11
    UTF7 = 11
    STRUCT = 12
    MAP = 13
    SET = 14
    LIST = 15
    UTF8 = 16
    UTF16 = 17
    FLOAT = 19


class TMessageType:
    CALL = 1
    REPLY = 2
    EXCEPTION = 3
    ONEWAY = 4


class TPriority:
    """apache::thrift::concurrency::PRIORITY"""

    HIGH_IMPORTANT = 0
    HIGH = 1
    IMPORTANT = 2
    NORMAL = 3
    BEST_EFFORT = 4
    N_PRIORITIES = 5


class TRequestContext:
    def __init__(self):
        self._headers = None

    def getHeaders(self):
        return self._headers

    def setHeaders(self, headers):
        self._headers = headers


class TProcessorEventHandler:
    """Event handler for thrift processors"""

    # TODO: implement asyncComplete for Twisted

    def getHandlerContext(self, fn_name, server_context):
        """Called at the start of processing a handler method"""
        return None

    def preRead(self, handler_context, fn_name, args):
        """Called before the handler method's argument are read"""
        pass

    def postRead(self, handler_context, fn_name, args):
        """Called after the handler method's argument are read"""
        pass

    def preWrite(self, handler_context, fn_name, result):
        """Called before the handler method's results are written"""
        pass

    def postWrite(self, handler_context, fn_name, result):
        """Called after the handler method's results are written"""
        pass

    def handlerException(self, handler_context, fn_name, exception):
        """Called if (and only if) the handler threw an expected exception."""
        pass

    def handlerError(self, handler_context, fn_name, exception):
        """Called if (and only if) the handler threw an unexpected exception.

        Note that this method is NOT called if the handler threw an
        exception that is declared in the thrift service specification"""

        logging.exception("Unexpected error in service handler " + fn_name + ":")


class TServerInterface:
    def __init__(self):
        self._tl_request_context = threading.local()

    def setRequestContext(self, request_context):
        self._tl_request_context.ctx = request_context

    def getRequestContext(self):
        return self._tl_request_context.ctx


class TProcessor:

    """Base class for processor, which works on two streams."""

    def __init__(self):
        self._event_handler = TProcessorEventHandler()  # null object handler
        self._handler = None
        self._processMap = {}
        self._priorityMap = {}

    def setEventHandler(self, event_handler):
        self._event_handler = event_handler

    def getEventHandler(self):
        return self._event_handler

    def process(self, iprot, oprot, server_context=None):
        pass

    def onewayMethods(self):
        return ()

    def readMessageBegin(self, iprot):
        name, _, seqid = iprot.readMessageBegin()
        if six.PY3:
            name = name.decode("utf8")
        return name, seqid

    def skipMessageStruct(self, iprot):
        iprot.skip(TType.STRUCT)
        iprot.readMessageEnd()

    def doesKnowFunction(self, name):
        return name in self._processMap

    def callFunction(self, name, seqid, iprot, oprot, server_ctx):
        process_fn = self._processMap[name]
        return process_fn(self, seqid, iprot, oprot, server_ctx)

    def readArgs(self, iprot, handler_ctx, fn_name, argtype):
        args = argtype()
        self._event_handler.preRead(handler_ctx, fn_name, args)
        args.read(iprot)
        iprot.readMessageEnd()
        self._event_handler.postRead(handler_ctx, fn_name, args)
        return args

    def writeException(self, oprot, name, seqid, exc):
        oprot.writeMessageBegin(name, TMessageType.EXCEPTION, seqid)
        exc.write(oprot)
        oprot.writeMessageEnd()
        oprot.trans.flush()

    def get_priority(self, fname):
        return self._priorityMap.get(fname, TPriority.NORMAL)

    def _getReplyType(self, result):
        if isinstance(result, TApplicationException):
            return TMessageType.EXCEPTION
        return TMessageType.REPLY

    @staticmethod
    def _get_exception_from_thrift_result(result):
        """Returns the wrapped exception, if pressent. None if not.

        result is a generated *_result object. This object either has a
        'success' field set indicating the call succeeded, or a field set
        indicating the exception thrown.
        """
        fields = (
            result.__dict__.keys() if hasattr(result, "__dict__") else result.__slots__
        )
        for field in fields:
            value = getattr(result, field)
            if value is None:
                continue
            elif field == "success":
                return None
            else:
                return value
        return None

    def writeReply(self, oprot, handler_ctx, fn_name, seqid, result, server_ctx=None):
        self._event_handler.preWrite(handler_ctx, fn_name, result)
        reply_type = self._getReplyType(result)

        if server_ctx is not None and hasattr(server_ctx, "context_data"):
            ex = (
                result
                if reply_type == TMessageType.EXCEPTION
                else self._get_exception_from_thrift_result(result)
            )
            if ex:
                server_ctx.context_data.setHeaderEx(ex.__class__.__name__)
                server_ctx.context_data.setHeaderExWhat(str(ex)[:UEXW_MAX_LENGTH])

        try:
            oprot.writeMessageBegin(fn_name, reply_type, seqid)
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
            self._event_handler.handlerError(handler_ctx, fn_name, e)

            # We raise the exception again to avoid any further processing
            raise

        finally:
            # Since we called preWrite, we should also call postWrite to
            # allow application to properly log their requests.
            self._event_handler.postWrite(handler_ctx, fn_name, result)


class TException(Exception):

    """Base class for all thrift exceptions."""

    # BaseException.message is deprecated in Python v[2.6,3.0)
    if (2, 6, 0) <= sys.version_info < (3, 0):

        def _get_message(self):
            return self._message

        def _set_message(self, message):
            self._message = message

        message = property(_get_message, _set_message)

    def __init__(self, message=None):
        Exception.__init__(self, message)
        self.message = message


class TApplicationException(TException):

    """Application level thrift exceptions."""

    UNKNOWN = 0
    UNKNOWN_METHOD = 1
    INVALID_MESSAGE_TYPE = 2
    WRONG_METHOD_NAME = 3
    BAD_SEQUENCE_ID = 4
    MISSING_RESULT = 5
    INTERNAL_ERROR = 6
    PROTOCOL_ERROR = 7
    INVALID_TRANSFORM = 8
    INVALID_PROTOCOL = 9
    UNSUPPORTED_CLIENT_TYPE = 10
    LOADSHEDDING = 11
    TIMEOUT = 12
    INJECTED_FAILURE = 13

    EXTYPE_TO_STRING = {
        UNKNOWN_METHOD: "Unknown method",
        INVALID_MESSAGE_TYPE: "Invalid message type",
        WRONG_METHOD_NAME: "Wrong method name",
        BAD_SEQUENCE_ID: "Bad sequence ID",
        MISSING_RESULT: "Missing result",
        INTERNAL_ERROR: "Internal error",
        PROTOCOL_ERROR: "Protocol error",
        INVALID_TRANSFORM: "Invalid transform",
        INVALID_PROTOCOL: "Invalid protocol",
        UNSUPPORTED_CLIENT_TYPE: "Unsupported client type",
        LOADSHEDDING: "Loadshedding request",
        TIMEOUT: "Task timeout",
        INJECTED_FAILURE: "Injected Failure",
    }

    def __init__(self, type=UNKNOWN, message=None):
        TException.__init__(self, message)
        self.type = type

    def __str__(self):
        if self.message:
            return self.message
        else:
            return self.EXTYPE_TO_STRING.get(
                self.type, "Default (unknown) TApplicationException"
            )

    def read(self, iprot):
        iprot.readStructBegin()
        while True:
            (fname, ftype, fid) = iprot.readFieldBegin()
            if ftype == TType.STOP:
                break
            if fid == 1:
                if ftype == TType.STRING:
                    message = iprot.readString()
                    if sys.version_info.major >= 3 and isinstance(message, bytes):
                        try:
                            message = message.decode("utf-8")
                        except UnicodeDecodeError:
                            pass
                    self.message = message
                else:
                    iprot.skip(ftype)
            elif fid == 2:
                if ftype == TType.I32:
                    self.type = iprot.readI32()
                else:
                    iprot.skip(ftype)
            else:
                iprot.skip(ftype)
            iprot.readFieldEnd()
        iprot.readStructEnd()

    def write(self, oprot):
        oprot.writeStructBegin(b"TApplicationException")
        if self.message is not None:
            oprot.writeFieldBegin(b"message", TType.STRING, 1)
            oprot.writeString(
                self.message.encode("utf-8")
                if not isinstance(self.message, bytes)
                else self.message
            )
            oprot.writeFieldEnd()
        if self.type is not None:
            oprot.writeFieldBegin(b"type", TType.I32, 2)
            oprot.writeI32(self.type)
            oprot.writeFieldEnd()
        oprot.writeFieldStop()
        oprot.writeStructEnd()


class UnimplementedTypedef:
    pass


def expand_thrift_spec(spec):
    """
    Given a sparse thrift_spec generate a full thrift_spec as expected by fastproto.
    The old form is a tuple where every position is the same as the thrift field id.
    The new form is just a tuple of the used field ids without all the None padding, but its cheaper bytecode wise.
    There is a bug in python 3.10 that causes large tuples to use more memory and generate larger .pyc than <=3.9.
    See: https://github.com/python/cpython/issues/109036
    """
    next_id = 0
    for item in spec:
        # there is some concept of negative field ids
        if next_id >= 0 and item[0] < 0:
            next_id = item[0]
        if item[0] != next_id:
            for _ in range(next_id, item[0]):
                yield None
        yield item
        next_id = item[0] + 1

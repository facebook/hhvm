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

import sys
import threading
import time
import traceback
from concurrent.futures import Future
from functools import partial

from thrift.protocol.THeaderProtocol import THeaderProtocol
from thrift.server.CppServerWrapper import (
    CallbackWrapper,
    CallTimestamps,
    CppContextData,
    CppServerWrapper,
    SSLPolicy,
    SSLVersion,
    VerifyClientCertificate,
)
from thrift.server.TServer import TConnectionContext, TServer
from thrift.Thrift import TPriority
from thrift.transport.THeaderTransport import MAX_BIG_FRAME_SIZE, THeaderTransport
from thrift.transport.TTransport import TMemoryBuffer

# Default sampling rate for expensive sampling operations, such as histogram
# counters.
kDefaultSampleRate = 32


class TCppConnectionContext(TConnectionContext):
    def __init__(self, context_data):
        self.context_data = context_data

    def getClientPrincipal(self):
        return self.context_data.getClientIdentity()

    def getClientPrincipalUser(self):
        principal = self.getClientPrincipal()
        if not principal:
            return None
        user, match, domain = principal.partition("@")
        if match:
            return user
        return None

    def getPeerName(self):
        return self.context_data.getPeerAddress()

    def getSockName(self):
        return self.context_data.getLocalAddress()


class _ProcessorAdapter(object):
    CONTEXT_DATA = CppContextData
    CALLBACK_WRAPPER = CallbackWrapper

    def __init__(self, processor):
        self.processor = processor
        self.observer = None
        self.sampleRate = kDefaultSampleRate
        self.sampleCount = 0

    def setObserver(self, observer):
        self.observer = observer

    def _shouldSample(self):
        self.sampleCount = (self.sampleCount + 1) % self.sampleRate
        return self.sampleCount == 0

    # TODO mhorowitz: add read headers here, so they can be added to
    # the constructed header buffer.  Also add endpoint addrs to the
    # context
    def call_processor(
        self, input, headers, client_type, protocol_type, context_data, callback
    ):
        try:
            # TCppServer threads are not created by Python so they are
            # missing settrace() hooks.  We need to manually set the
            # hook here for things to work (e.g. coverage and pdb).
            if sys.gettrace() is None and threading._trace_hook is not None:
                sys.settrace(threading._trace_hook)

            # The input string has already had the header removed, but
            # the python processor will expect it to be there.  In
            # order to reconstitute the message with headers, we use
            # the THeaderProtocol object to write into a memory
            # buffer, then pass that buffer to the python processor.

            should_sample = self._shouldSample()

            timestamps = CallTimestamps()
            if self.observer and should_sample:
                timestamps.setProcessBeginNow()

            write_buf = TMemoryBuffer()
            trans = THeaderTransport(write_buf)
            trans.set_max_frame_size(MAX_BIG_FRAME_SIZE)
            trans._THeaderTransport__client_type = client_type
            trans._THeaderTransport__write_headers = headers
            trans.set_protocol_id(protocol_type)
            trans.write(input)
            trans.flush()

            prot_buf = TMemoryBuffer(write_buf.getvalue())
            prot = THeaderProtocol(prot_buf, client_types=[client_type])
            prot.trans.set_max_frame_size(MAX_BIG_FRAME_SIZE)

            ctx = TCppConnectionContext(context_data)

            ret = self.processor.process(prot, prot, ctx)

            done_callback = partial(
                _ProcessorAdapter.done,
                prot_buf=prot_buf,
                client_type=client_type,
                callback=callback,
            )

            if self.observer:
                if should_sample:
                    timestamps.setProcessEndNow()

                # This only bumps counters if `processBegin != 0` and
                # `processEnd != 0` and these will only be non-zero if
                # we are sampling this request.
                self.observer.callCompleted(timestamps)

            # This future is created by and returned from the processor's
            # ThreadPoolExecutor, which keeps a reference to it. So it is
            # fine for this future to end its lifecycle here.
            if isinstance(ret, Future):
                ret.add_done_callback(lambda x, d=done_callback: d())
            else:
                done_callback()
        except:  # noqa
            # Don't let exceptions escape back into C++
            traceback.print_exc()

    @staticmethod
    def done(prot_buf, client_type, callback):
        try:
            response = prot_buf.getvalue()

            if len(response) == 0:
                callback.call(response)
            else:
                # And on the way out, we need to strip off the header,
                # because the C++ code will expect to add it.

                read_buf = TMemoryBuffer(response)
                trans = THeaderTransport(read_buf, client_types=[client_type])
                trans.set_max_frame_size(MAX_BIG_FRAME_SIZE)
                trans.readFrame(len(response))
                callback.call(trans.cstringio_buf.read())
        except:  # noqa
            # Don't let exceptions escape back into C++
            traceback.print_exc()

    def oneway_methods(self):
        return self.processor.onewayMethods()

    def get_priority(self, fname):
        try:
            return self.processor.get_priority(fname)
        except:  # noqa
            traceback.print_exc()
            return TPriority.NORMAL


class TSSLConfig(object):
    def __init__(self):
        self.cert_path = ""
        self.key_path = ""
        self.key_pw_path = ""
        self.client_ca_path = ""
        self.ecc_curve_name = ""
        self.verify = VerifyClientCertificate.IF_PRESENTED
        self.ssl_policy = SSLPolicy.REQUIRED
        self.ticket_file_path = ""
        self.session_context = None
        self.ssl_version = None

    @property
    def ssl_version(self):
        return self._ssl_version

    @ssl_version.setter
    def ssl_version(self, val):
        if val is not None and not isinstance(val, SSLVersion):
            raise ValueError("{} is an invalid version".format(val))
        self._ssl_version = val

    @property
    def ssl_policy(self):
        return self._ssl_policy

    @ssl_policy.setter
    def ssl_policy(self, val):
        if not isinstance(val, SSLPolicy):
            raise ValueError("{} is an invalid policy".format(val))
        self._ssl_policy = val

    @property
    def verify(self):
        return self._verify

    @verify.setter
    def verify(self, val):
        if not isinstance(val, VerifyClientCertificate):
            raise ValueError("{} is an invalid value".format(val))
        self._verify = val


class TSSLCacheOptions(object):
    def __init__(self):
        self.ssl_cache_timeout_seconds = 86400
        self.max_ssl_cache_size = 20480
        self.ssl_cache_flush_size = 200
        self.ssl_handshake_validity_seconds = 259200


class TCppServer(CppServerWrapper, TServer):
    def __init__(self, processor):
        CppServerWrapper.__init__(self)
        self.processor = self._getProcessor(processor)
        self.setAdapter(_ProcessorAdapter(self.processor))
        self._setup_done = False
        self.serverEventHandler = None
        self.setWrapperName("TCppServer-py")

    def setAdapter(self, adapter):
        self.processorAdapter = adapter
        CppServerWrapper.setAdapter(self, adapter)

    def setObserver(self, observer):
        self.processorAdapter.setObserver(observer)
        CppServerWrapper.setObserver(self, observer)

    def setServerEventHandler(self, handler):
        TServer.setServerEventHandler(self, handler)
        handler.CONTEXT_DATA = CppContextData
        handler.CPP_CONNECTION_CONTEXT = TCppConnectionContext
        self.setCppServerEventHandler(handler)

    def setSSLConfig(self, config):
        if not isinstance(config, TSSLConfig):
            raise ValueError("Config must be of type TSSLConfig")
        self.setCppSSLConfig(config)

    def setSSLCacheOptions(self, cache_options):
        if not isinstance(cache_options, TSSLCacheOptions):
            raise ValueError("Options might be of type TSSLCacheOptions")
        self.setCppSSLCacheOptions(cache_options)

    def setFastOpenOptions(self, enabled, tfo_max_queue):
        self.setCppFastOpenOptions(enabled, tfo_max_queue)

    def useExistingSocket(self, socket):
        self.useCppExistingSocket(socket)

    def getTicketSeeds(self):
        return self.getCppTicketSeeds()

    def setup(self):
        if self._setup_done:
            return
        CppServerWrapper.setup(self)
        if self.serverEventHandler is not None:
            self.serverEventHandler.preServe(self.getAddress())
        self._setup_done = True

    def loop(self):
        if not self._setup_done:
            raise RuntimeError("setup() must be called before loop()")
        CppServerWrapper.loop(self)

    def serve(self):
        self.setup()
        try:
            self.loop()
        finally:
            self.cleanUp()

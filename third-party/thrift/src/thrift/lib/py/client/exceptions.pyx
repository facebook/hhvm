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

from cython.operator cimport dereference as deref
from thrift.transport.TTransport import TTransportException
from thrift.Thrift import TApplicationException


cdef object create_ApplicationError(unique_ptr[cTApplicationException] ex):
    if not ex:
        return
    type = deref(ex).getType()
    message = (<bytes>deref(ex).what()).decode('utf-8')
    # Strip out the message prefix its verbose for python
    message = message[message.startswith('TApplicationException: ')*23:]
    return TApplicationException(type, message)


cdef object create_TransportError(unique_ptr[cTTransportException] ex):
    if not ex:
        return
    type = deref(ex).getType()
    message = (<bytes>deref(ex).what()).decode('utf-8')
    # Strip off the c++ message prefix
    message = message[message.startswith('TTransportException: ')*21:]
    return TTransportException(type, message)


cdef object create_py_exception(const cFollyExceptionWrapper& ex):
    pyex = create_ApplicationError(try_make_unique_exception[cTApplicationException](ex))
    if pyex:
        return pyex

    pyex = create_TransportError(try_make_unique_exception[cTTransportException](ex))
    if pyex:
        return pyex

    try:
        # No clue what this is just throw it and let the default cython logic takeover
        ex.throw_exception()
    except Exception as pyex:
        # We don't try to shorten the traceback because this is Unknown
        # it will be helpful to know the most information possible.
        return pyex

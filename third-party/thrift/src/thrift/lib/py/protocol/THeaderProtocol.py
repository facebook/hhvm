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

from thrift.protocol.TBinaryProtocol import TBinaryProtocolAccelerated
from thrift.protocol.TCompactProtocol import TCompactProtocolAccelerated
from thrift.protocol.TProtocol import TProtocolBase, TProtocolException
from thrift.Thrift import TApplicationException, TMessageType
from thrift.transport.THeaderTransport import CLIENT_TYPE, THeaderTransport


class THeaderProtocolAccelerate:

    """Base class for pass through header protocols"""

    def get_protocol_id(self):
        raise NotImplementedError()


class THeaderProtocol(TProtocolBase, THeaderProtocolAccelerate):

    """Pass through header protocol (transport can set)"""

    T_BINARY_PROTOCOL = 0
    T_JSON_PROTOCOL = 1
    T_COMPACT_PROTOCOL = 2

    __proto = None
    __proto_id = None

    def get_protocol_id(self):
        return self.__proto_id

    def reset_protocol(self):
        if self.__proto_id == self.trans.get_protocol_id():
            return

        proto_id = self.trans.get_protocol_id()

        if proto_id == self.T_BINARY_PROTOCOL:
            self.__proto = TBinaryProtocolAccelerated(self.trans, self.strictRead, True)
        elif proto_id == self.T_COMPACT_PROTOCOL:
            self.__proto = TCompactProtocolAccelerated(self.trans)
        else:
            raise TApplicationException(
                TProtocolException.INVALID_PROTOCOL, "Unknown protocol requested"
            )
        self.__proto_id = proto_id

    def __init__(self, trans, strictRead=False, client_types=None, client_type=None):
        """Create a THeaderProtocol instance

        @param transport(TTransport) The underlying transport.
        @param strictRead(bool) Turn on strictRead if using TBinaryProtocol
        @param client_types([CLIENT_TYPE.HEADER, ...])
                   List of client types to support.  Defaults to
                   CLIENT_TYPE.HEADER only.
        """

        if isinstance(trans, THeaderTransport):
            trans._THeaderTransport__supported_client_types = set(
                client_types or (CLIENT_TYPE.HEADER,)
            )
            if client_type is not None:
                trans._THeaderTransport__client_type = client_type
            htrans = trans
        else:
            htrans = THeaderTransport(trans, client_types, client_type)
        TProtocolBase.__init__(self, htrans)
        self.strictRead = strictRead
        self.reset_protocol()

    def writeMessageBegin(self, name, type, seqid):
        self.__proto.writeMessageBegin(name, type, seqid)
        if type == TMessageType.CALL or type == TMessageType.ONEWAY:
            # All client to server coms should have a unique seq_id in HEADER
            self.trans.seq_id = seqid

    def writeMessageEnd(self):
        self.__proto.writeMessageEnd()

    def writeStructBegin(self, name):
        self.__proto.writeStructBegin(name)

    def writeStructEnd(self):
        self.__proto.writeStructEnd()

    def writeFieldBegin(self, name, type, id):
        self.__proto.writeFieldBegin(name, type, id)

    def writeFieldEnd(self):
        self.__proto.writeFieldEnd()

    def writeFieldStop(self):
        self.__proto.writeFieldStop()

    def writeMapBegin(self, ktype, vtype, size):
        self.__proto.writeMapBegin(ktype, vtype, size)

    def writeMapEnd(self):
        self.__proto.writeMapEnd()

    def writeListBegin(self, etype, size):
        self.__proto.writeListBegin(etype, size)

    def writeListEnd(self):
        self.__proto.writeListEnd()

    def writeSetBegin(self, etype, size):
        self.__proto.writeSetBegin(etype, size)

    def writeSetEnd(self):
        self.__proto.writeSetEnd()

    def writeBool(self, bool):
        self.__proto.writeBool(bool)

    def writeByte(self, byte):
        self.__proto.writeByte(byte)

    def writeI16(self, i16):
        self.__proto.writeI16(i16)

    def writeI32(self, i32):
        self.__proto.writeI32(i32)

    def writeI64(self, i64):
        self.__proto.writeI64(i64)

    def writeDouble(self, dub):
        self.__proto.writeDouble(dub)

    def writeFloat(self, flt):
        self.__proto.writeFloat(flt)

    def writeString(self, str):
        self.__proto.writeString(str)

    def readMessageBegin(self):
        # Read the next frame, and change protocols if needed
        try:
            self.trans._reset_protocol()
            self.reset_protocol()
        except TApplicationException as ex:
            if self.__proto:
                self.writeMessageBegin(b"", TMessageType.EXCEPTION, 0)
                ex.write(self)
                self.writeMessageEnd()
                self.trans.flush()
        return self.__proto.readMessageBegin()

    def readMessageEnd(self):
        return self.__proto.readMessageEnd()

    def readStructBegin(self):
        return self.__proto.readStructBegin()

    def readStructEnd(self):
        return self.__proto.readStructEnd()

    def readFieldBegin(self):
        return self.__proto.readFieldBegin()

    def readFieldEnd(self):
        return self.__proto.readFieldEnd()

    def readMapBegin(self):
        return self.__proto.readMapBegin()

    def readMapEnd(self):
        return self.__proto.readMapEnd()

    def readListBegin(self):
        return self.__proto.readListBegin()

    def readListEnd(self):
        return self.__proto.readListEnd()

    def readSetBegin(self):
        return self.__proto.readSetBegin()

    def readSetEnd(self):
        return self.__proto.readSetEnd()

    def readBool(self):
        return self.__proto.readBool()

    def readByte(self):
        return self.__proto.readByte()

    def readI16(self):
        return self.__proto.readI16()

    def readI32(self):
        return self.__proto.readI32()

    def readI64(self):
        return self.__proto.readI64()

    def readDouble(self):
        return self.__proto.readDouble()

    def readFloat(self):
        return self.__proto.readFloat()

    def readString(self):
        return self.__proto.readString()


class THeaderProtocolFactory:
    def __init__(self, strictRead=False, client_types=None, client_type=None):
        self.strictRead = strictRead
        self.client_types = client_types
        self.client_type = client_type

    def getProtocol(self, trans):
        prot = THeaderProtocol(
            trans, self.strictRead, self.client_types, self.client_type
        )
        return prot

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

import json
import sys
from base64 import b64decode, b64encode

from thrift.protocol.TProtocol import TProtocolBase, TProtocolException
from thrift.Thrift import TType

JSON_OBJECT_START = b"{"
JSON_OBJECT_END = b"}"
JSON_ARRAY_START = b"["
JSON_ARRAY_END = b"]"
JSON_NEW_LINE = b"\n"
JSON_PAIR_SEPARATOR = b":"
JSON_ELEM_SEPARATOR = b","
JSON_BACKSLASH = b"\\"
JSON_BACKSLASH_VALUE = ord(JSON_BACKSLASH)
JSON_STRING_DELIMITER = b'"'
JSON_ZERO_CHAR = b"0"
JSON_TAB = b"  "
JSON_CARRIAGE_RETURN = b"\r"
JSON_SPACE = b" "
TAB = b"\t"

JSON_ESCAPE_CHAR = b"u"
JSON_ESCAPE_PREFIX = b"\\u00"

THRIFT_VERSION_1 = 1

THRIFT_NAN = b"NaN"
THRIFT_INFINITY = b"Infinity"
THRIFT_NEGATIVE_INFINITY = b"-Infinity"

JSON_CHAR_TABLE = [  #   0   1    2   3   4   5   6   7    8    9    A   B    C    D   E   F
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    b"b",
    b"t",
    b"n",
    0,
    b"f",
    b"r",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    b'"',
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
]


JSON_CHARS_TO_ESCAPE = set()
for ch_value, mode in enumerate(JSON_CHAR_TABLE):
    if mode == 1:
        continue
    if sys.version_info[0] == 3:
        JSON_CHARS_TO_ESCAPE.add(chr(ch_value).encode("ascii"))
        JSON_CHARS_TO_ESCAPE.add(chr(ch_value))
    else:
        JSON_CHARS_TO_ESCAPE.add(chr(ch_value))
        JSON_CHARS_TO_ESCAPE.add(chr(ch_value).encode("utf-8"))
JSON_CHARS_TO_ESCAPE.add(JSON_BACKSLASH)
JSON_CHARS_TO_ESCAPE.add(JSON_BACKSLASH.decode("utf-8"))

ESCAPE_CHARS = b'"\\bfnrt'
ESCAPE_CHAR_VALS = [b'"', b"\\", b"\b", b"\f", b"\n", b"\r", b"\t"]

NUMERIC_CHAR = b"+-.0123456789Ee"

WHITESPACE_CHARS = {
    JSON_NEW_LINE,
    TAB,
    JSON_CARRIAGE_RETURN,
    JSON_SPACE,
}


def hexChar(x):
    x &= 0x0F
    return hex(x)[2:]


def hexVal(ch):
    if ch >= "0" and ch <= "9":
        return int(ch) - int("0")
    elif ch >= "a" and ch <= "f":
        return int(ch) - int("a") + 10
    raise TProtocolException(TProtocolException.INVALID_DATA, "Unexpected hex value")


class TJSONContext:
    def __init__(self, protocol, indentLevel=0):
        self.indentLevel = indentLevel
        self.protocol = protocol

    def write(self, trans):
        return

    def read(self, reader):
        return

    def escapeNum(self):
        return False

    def writeNewLine(self, trans):
        trans.write(JSON_NEW_LINE)
        self.indent(trans)

    def indent(self, trans):
        trans.write(JSON_TAB * self.indentLevel)


class TJSONPairContext(TJSONContext):
    def __init__(self, protocol, indentLevel=0, isMapPair=False):
        TJSONContext.__init__(self, protocol, indentLevel)
        self.first = True
        self.colon = True
        self.isMapPair = isMapPair
        self.skipColon = False

    def write(self, trans):
        if self.first:
            self.first = False
            self.colon = True
        else:
            if self.colon:
                trans.write(JSON_PAIR_SEPARATOR + b" ")
            else:
                trans.write(JSON_ELEM_SEPARATOR)
                if self.isMapPair:
                    self.writeNewLine(trans)
            self.colon = not self.colon

    def read(self, reader):
        if self.first:
            self.first = False
            self.colon = True
        else:
            self.protocol.skipWhitespace()
            if self.colon:
                if self.skipColon:
                    self.skipColon = False
                else:
                    self.protocol.readJSONSyntaxChar(JSON_PAIR_SEPARATOR)
            else:
                self.protocol.readJSONSyntaxChar(JSON_ELEM_SEPARATOR)
            self.colon = not self.colon

    def escapeNum(self):
        return self.colon


class TJSONListContext(TJSONContext):
    def __init__(self, protocol, indentLevel=0):
        TJSONContext.__init__(self, protocol, indentLevel)
        self.first = True

    def read(self, reader):
        if self.first:
            self.first = False
        else:
            self.protocol.skipWhitespace()
            self.protocol.readJSONSyntaxChar(JSON_ELEM_SEPARATOR)

    def write(self, trans):
        if self.first:
            self.first = False
        else:
            trans.write(JSON_ELEM_SEPARATOR)
            self.writeNewLine(trans)


class LookaheadReader:
    def __init__(self, protocol):
        self.protocol = protocol
        self.hasData = False
        self.data = b""

    def read(self):
        if self.hasData is True:
            self.hasData = False
        else:
            self.data = self.protocol.trans.read(1)
        return self.data

    def peek(self):
        if self.hasData is False:
            self.data = self.protocol.trans.read(1)
            self.hasData = True
        return self.data


class ThriftSpec:
    def __init__(self, spec):
        self.spec = spec
        self.nextSpec = None


class StructSpec(ThriftSpec):
    """
    Wraps thrift_spec of a thrift struct.
    """

    def readFieldBegin(self, fname, guess_func):
        field_spec = None
        self.nextSpec = None
        if sys.version_info[0] >= 3:
            fname = fname.decode()
        for s in self.spec:
            if s is not None and s[2] == fname:
                field_spec = s
                break

        if field_spec is not None:
            if field_spec[1] == TType.STRUCT:
                self.nextSpec = StructSpec(field_spec[3][1])
            elif field_spec[1] in (TType.SET, TType.LIST):
                self.nextSpec = ListOrSetSpec(field_spec[3])
            elif field_spec[1] == TType.MAP:
                self.nextSpec = MapSpec(field_spec[3])
            return (fname, field_spec[1], field_spec[0])
        else:
            return (fname, guess_func(), 0)

    def getNextSpec(self):
        return self.nextSpec


class ListOrSetSpec(ThriftSpec):
    """Wraps a list or set's 2-tuple nested type spec.

    getNextSpec is called in readListBegin to *prepare* the spec of
    the list element which may/may not be used depending on whether
    the list is empty.

    For example, to read list<SomeStruct> the following methods will
    be called:
        readListBegin()
            readStructBegin()
            readStructEnd()
            ...
        readListEnd()
    After readListBegin is called the current spec is still
    ListOrSetSpec and its nextSpec is prepared for its element.
    readStructBegin/End will push/pop the element's StructSpec
    whenever a SomeStruct is read.

    -1 tells the generated code that the size of this list is
    undetermined so it needs to use peekList to detect the end of
    the list.
    """

    def readListBegin(self):
        self.getNextSpec()
        return (self.spec[0], -1)

    readSetBegin = readListBegin

    def getNextSpec(self):
        if self.nextSpec is None:
            if self.spec[0] == TType.STRUCT:
                self.nextSpec = StructSpec(self.spec[1][1])
            elif self.spec[0] in (TType.LIST, TType.SET):
                self.nextSpec = ListOrSetSpec(self.spec[1])
            elif self.spec[0] == TType.MAP:
                self.nextSpec = MapSpec(self.spec[1])
        return self.nextSpec


class MapSpec(ThriftSpec):
    """Wraps a map's 4-tuple key/vale type spec."""

    def __init__(self, spec):
        ThriftSpec.__init__(self, spec)
        self.key = True

        self.keySpec = None
        if self.spec[1] is not None:
            if self.spec[0] == TType.STRUCT:
                self.keySpec = StructSpec(self.spec[1][1])
            elif self.spec[0] in (TType.LIST, TType.SET):
                self.keySpec = ListOrSetSpec(self.spec[1])
            elif self.spec[0] == TType.MAP:
                self.keySpec = MapSpec(self.spec[1])

        self.valueSpec = None
        if self.spec[3] is not None:
            if self.spec[2] == TType.STRUCT:
                self.valueSpec = StructSpec(self.spec[3][1])
            elif self.spec[2] in (TType.LIST, TType.SET):
                self.valueSpec = ListOrSetSpec(self.spec[3])
            elif self.spec[2] == TType.MAP:
                self.valueSpec = MapSpec(self.spec[3])

    def readMapBegin(self):
        self.getNextSpec()
        return (self.spec[0], self.spec[2], -1)

    def getNextSpec(self):
        if self.keySpec is not None and self.valueSpec is not None:
            self.nextSpec = self.keySpec if self.key is True else self.valueSpec
            self.key = not self.key
        else:
            self.nextSpec = self.keySpec if self.keySpec is not None else self.valueSpec

        return self.nextSpec


class TSimpleJSONProtocolBase(TProtocolBase, object):
    def __init__(self, trans, spec=None):
        TProtocolBase.__init__(self, trans)
        # Used as stack for contexts.
        self.contexts = [TJSONContext(protocol=self)]
        self.context = TJSONContext(protocol=self)
        self.pair_context_class = TJSONPairContext
        self.list_context_class = TJSONListContext
        self.reader = LookaheadReader(self)
        self.specs = []
        self.spec = StructSpec(spec)

    def pushContext(self, newContext):
        self.contexts.append(self.context)
        self.context = newContext

    def popContext(self):
        if len(self.contexts) > 0:
            self.context = self.contexts.pop()

    def pushSpec(self, newSpec):
        self.specs.append(self.spec)
        self.spec = newSpec

    def popSpec(self):
        if len(self.specs) > 0:
            self.spec = self.specs.pop()

    def skipWhitespace(self):
        skipped = 0
        while True:
            ch = self.reader.peek()
            if ch not in WHITESPACE_CHARS:
                break
            self.reader.read()
            skipped += 1
        return skipped

    def skip(self, _type):
        self.context.read(self.reader)
        self.skipWhitespace()
        type = self.guessTypeIdFromFirstByte()
        # Since self.context.read is called at the beginning of all readJSONxxx
        # methods and we have already called it here, push an empty context so that
        # it becomes a no-op.
        self.pushContext(TJSONContext(protocol=self))
        if type == TType.STRUCT:
            self.readJSONObjectStart()
            while True:
                (_, ftype, _) = self.readFieldBegin()
                if ftype == TType.STOP:
                    break
                self.skip(TType.VOID)
            self.readJSONObjectEnd()
        elif type == TType.LIST:
            self.readJSONArrayStart()
            while self.peekList():
                self.skip(TType.VOID)
            self.readJSONArrayEnd()
        elif type == TType.STRING:
            self.readJSONString()
        elif type == TType.DOUBLE:
            self.readJSONDouble()
        elif type == TType.BOOL:
            self.readJSONBool()
        else:
            raise TProtocolException(
                TProtocolException.INVALID_DATA,
                "Unexpected type {} guessed when skipping".format(type),
            )
        self.popContext()

    def guessTypeIdFromFirstByte(self):
        self.skipWhitespace()
        byte = self.reader.peek()
        if byte == JSON_OBJECT_END or byte == JSON_ARRAY_END:
            return TType.STOP
        elif byte == JSON_STRING_DELIMITER:
            return TType.STRING
        elif byte == JSON_OBJECT_START:
            return TType.STRUCT
        elif byte == JSON_ARRAY_START:
            return TType.LIST
        elif byte == b"t" or byte == b"f":
            return TType.BOOL
        elif byte in (
            b"+",
            b"-",
            b"0",
            b"1",
            b"2",
            b"3",
            b"4",
            b"5",
            b"6",
            b"7",
            b"8",
            b"9",
        ):
            return TType.DOUBLE
        else:
            raise TProtocolException(
                TProtocolException.INVALID_DATA, "Unrecognized byte: {}".format(byte)
            )

    def writeJSONEscapeChar(self, ch):
        self.trans.write(JSON_ESCAPE_PREFIX)
        self.trans.write(hexChar(ch >> 4))
        self.trans.write(hexChar(ch))

    def writeJSONChar(self, ch):
        charValue = ord(ch)
        if charValue >= 0x30:
            # The only special character >= 0x30 is '\'.
            if charValue == JSON_BACKSLASH_VALUE:
                self.trans.write(JSON_BACKSLASH)
                self.trans.write(JSON_BACKSLASH)
            else:
                self.trans.write(ch)
        else:
            outCh = JSON_CHAR_TABLE[charValue]
            if outCh == 1:
                self.trans.write(ch)
            elif outCh:
                self.trans.write(JSON_BACKSLASH)
                self.trans.write(outCh)
            else:
                self.writeJSONEscapeChar(charValue)

    def writeJSONString(self, outStr):
        self.context.write(self.trans)
        self.trans.write(JSON_STRING_DELIMITER)
        outStrLen = len(outStr)
        if outStrLen > 0:
            is_int = isinstance(outStr[0], int)
            pos = 0
            for idx, ch in enumerate(outStr):
                if is_int:
                    ch = outStr[idx : idx + 1]
                if ch in JSON_CHARS_TO_ESCAPE:
                    if pos < idx:
                        # Write previous chunk not requiring escaping
                        self.trans.write(outStr[pos:idx])
                    # Write current char with escaping
                    self.writeJSONChar(ch)
                    # Advance pos
                    pos = idx + 1
            if pos < outStrLen:
                # Write last chunk till outStrLen
                self.trans.write(outStr[pos:outStrLen])
        self.trans.write(JSON_STRING_DELIMITER)

    def writeJSONBase64(self, outStr):
        self.context.write(self.trans)
        self.trans.write(JSON_STRING_DELIMITER)
        b64Str = b64encode(outStr)
        self.trans.write(b64Str)
        self.trans.write(JSON_STRING_DELIMITER)

    def writeJSONInteger(self, num):
        self.context.write(self.trans)
        escapeNum = self.context.escapeNum()
        numStr = str(num)
        if escapeNum:
            self.trans.write(JSON_STRING_DELIMITER)
        self.trans.write(numStr)
        if escapeNum:
            self.trans.write(JSON_STRING_DELIMITER)

    def writeJSONBool(self, boolVal):
        self.context.write(self.trans)
        if self.context.escapeNum():
            self.trans.write(JSON_STRING_DELIMITER)
        if boolVal:
            self.trans.write(b"true")
        else:
            self.trans.write(b"false")
        if self.context.escapeNum():
            self.trans.write(JSON_STRING_DELIMITER)

    def writeJSONDouble(self, num):
        self.context.write(self.trans)
        numStr = str(num)
        special = False

        if numStr == "nan":
            numStr = THRIFT_NAN
            special = True
        elif numStr == "inf":
            numStr = THRIFT_INFINITY
            special = True
        elif numStr == "-inf":
            numStr = THRIFT_NEGATIVE_INFINITY
            special = True

        escapeNum = special or self.context.escapeNum()
        if escapeNum:
            self.trans.write(JSON_STRING_DELIMITER)
        self.trans.write(numStr)
        if escapeNum:
            self.trans.write(JSON_STRING_DELIMITER)

    def writeJSONObjectStart(self):
        self.context.write(self.trans)
        self.trans.write(JSON_OBJECT_START)
        self.pushContext(
            self.pair_context_class(protocol=self, indentLevel=len(self.contexts))
        )

    def writeJSONObjectEnd(self):
        self.popContext()
        self.context.writeNewLine(self.trans)
        self.trans.write(JSON_OBJECT_END)

    def writeJSONArrayStart(self):
        self.context.write(self.trans)
        self.trans.write(JSON_ARRAY_START)
        self.pushContext(
            self.list_context_class(protocol=self, indentLevel=len(self.contexts))
        )

    def writeJSONArrayEnd(self):
        self.popContext()
        self.context.writeNewLine(self.trans)
        self.trans.write(JSON_ARRAY_END)

    def writeJSONMapStart(self):
        self.context.write(self.trans)
        self.trans.write(JSON_OBJECT_START)
        self.pushContext(
            self.list_context_class(protocol=self, indentLevel=len(self.contexts))
        )

    def writeJSONMapEnd(self):
        self.popContext()
        self.context.writeNewLine(self.trans)
        self.trans.write(JSON_OBJECT_END)

    def readJSONSyntaxChar(self, char):
        ch = self.reader.read()
        if ch != char:
            raise TProtocolException(
                TProtocolException.INVALID_DATA, "Unexpected character: %s" % ch
            )

    def readJSONString(self, skipContext=False):
        self.skipWhitespace()
        if skipContext is False:
            self.context.read(self.reader)
        self.skipWhitespace()
        self.readJSONSyntaxChar(JSON_STRING_DELIMITER)
        string = []
        while True:
            ch = self.reader.read()
            if ch == JSON_STRING_DELIMITER:
                break
            if ch == JSON_BACKSLASH:
                ch = self.reader.read()
                if ch == b"u":
                    self.readJSONSyntaxChar(JSON_ZERO_CHAR)
                    self.readJSONSyntaxChar(JSON_ZERO_CHAR)
                    data = self.trans.read(2)
                    if sys.version_info[0] >= 3 and isinstance(data, bytes):
                        ch = (
                            json.JSONDecoder()
                            .decode('"\\u00%s"' % str(data, "utf-8"))
                            .encode("utf-8")
                        )
                    else:
                        ch = json.JSONDecoder().decode('"\\u00%s"' % data)
                else:
                    idx = ESCAPE_CHARS.find(ch)
                    if idx == -1:
                        raise TProtocolException(
                            TProtocolException.INVALID_DATA, "Expected control char"
                        )
                    ch = ESCAPE_CHAR_VALS[idx]
            string.append(ch)
        return b"".join(string)

    def isJSONNumeric(self, ch):
        return NUMERIC_CHAR.find(ch) >= 0

    def readJSONNumericChars(self):
        numeric = []
        while True:
            ch = self.reader.peek()
            if self.isJSONNumeric(ch) is False:
                break
            numeric.append(self.reader.read())
        return b"".join(numeric)

    def readJSONInteger(self):
        self.context.read(self.reader)
        self.skipWhitespace()
        if self.context.escapeNum():
            self.readJSONSyntaxChar(JSON_STRING_DELIMITER)
        numeric = self.readJSONNumericChars()
        if self.context.escapeNum():
            self.readJSONSyntaxChar(JSON_STRING_DELIMITER)
        try:
            return int(numeric)
        except ValueError:
            raise TProtocolException(
                TProtocolException.INVALID_DATA, "Bad data encounted in numeric data"
            )

    def readJSONDouble(self):
        self.context.read(self.reader)
        self.skipWhitespace()
        if self.reader.peek() == JSON_STRING_DELIMITER:
            string = self.readJSONString(True)
            try:
                double = float(string)
                if (
                    self.context.escapeNum is False
                    and double != float("inf")
                    and double != float("-inf")
                    and double != float("nan")
                ):
                    raise TProtocolException(
                        TProtocolException.INVALID_DATA,
                        "Numeric data unexpectedly quoted",
                    )
                return double
            except ValueError:
                raise TProtocolException(
                    TProtocolException.INVALID_DATA,
                    "Bad data encountered in numeric data",
                )
        else:
            try:
                return float(self.readJSONNumericChars())
            except ValueError:
                raise TProtocolException(
                    TProtocolException.INVALID_DATA,
                    "Bad data encountered in numeric data",
                )

    def readJSONBase64(self):
        string = self.readJSONString()
        return b64decode(string)

    def readJSONBool(self):
        self.context.read(self.reader)
        self.skipWhitespace()
        if self.context.escapeNum():
            self.readJSONSyntaxChar(JSON_STRING_DELIMITER)
        if self.reader.peek() == b"t":
            true_string = b"true"
            for i in range(4):
                if self.reader.read() != true_string[i : i + 1]:
                    raise TProtocolException(
                        TProtocolException.INVALID_DATA, "Bad data encountered in bool"
                    )
            boolVal = True
        elif self.reader.peek() == b"f":
            false_string = b"false"
            for i in range(5):
                if self.reader.read() != false_string[i : i + 1]:
                    raise TProtocolException(
                        TProtocolException.INVALID_DATA, "Bad data encountered in bool"
                    )
            boolVal = False
        else:
            raise TProtocolException(
                TProtocolException.INVALID_DATA, "Bad data encountered in bool"
            )
        if self.context.escapeNum():
            self.readJSONSyntaxChar(JSON_STRING_DELIMITER)
        return boolVal

    def readJSONArrayStart(self):
        self.context.read(self.reader)
        self.skipWhitespace()
        self.readJSONSyntaxChar(JSON_ARRAY_START)
        self.pushContext(
            self.list_context_class(protocol=self, indentLevel=len(self.contexts))
        )

    def readJSONArrayEnd(self):
        self.popContext()
        self.skipWhitespace()
        self.readJSONSyntaxChar(JSON_ARRAY_END)

    def readJSONMapStart(self):
        self.context.read(self.reader)
        self.skipWhitespace()
        self.readJSONSyntaxChar(JSON_OBJECT_START)
        self.pushContext(
            self.list_context_class(protocol=self, indentLevel=len(self.contexts))
        )

    def readJSONMapEnd(self):
        self.popContext()
        self.skipWhitespace()
        self.readJSONSyntaxChar(JSON_OBJECT_END)

    def readJSONObjectStart(self):
        self.context.read(self.reader)
        self.skipWhitespace()
        self.readJSONSyntaxChar(JSON_OBJECT_START)
        self.pushContext(
            self.pair_context_class(protocol=self, indentLevel=len(self.contexts))
        )

    def readJSONObjectEnd(self):
        self.popContext()
        self.skipWhitespace()
        self.readJSONSyntaxChar(JSON_OBJECT_END)


class TSimpleJSONProtocol(TSimpleJSONProtocolBase):
    """
    JSON protocol implementation for Thrift. This protocol is write-only, and
    produces a simple output format that conforms to the JSON standard.
    """

    def writeMessageBegin(self, name, messageType, seqId):
        self.writeJSONArrayStart()
        self.context.writeNewLine(self.trans)
        self.writeJSONInteger(THRIFT_VERSION_1)
        self.writeJSONString(name)
        self.writeJSONInteger(messageType)
        self.writeJSONInteger(seqId)

    def writeMessageEnd(self):
        self.writeJSONArrayEnd()

    def writeStructBegin(self, name):
        self.writeJSONObjectStart()

    def writeStructEnd(self):
        self.writeJSONObjectEnd()

    def writeFieldBegin(self, name, fieldType, fieldId):
        self.context.write(self.trans)
        self.popContext()
        self.pushContext(
            self.pair_context_class(protocol=self, indentLevel=len(self.contexts))
        )
        self.context.writeNewLine(self.trans)
        self.writeJSONString(name)

    def writeFieldEnd(self):
        return

    def writeFieldStop(self):
        return

    def writeMapBegin(self, keyType, valType, size):
        self.writeJSONMapStart()
        self.context.writeNewLine(self.trans)
        self.pushContext(
            self.pair_context_class(
                protocol=self, indentLevel=len(self.contexts) - 1, isMapPair=True
            )
        )

    def writeMapEnd(self):
        self.popContext()
        self.writeJSONMapEnd()

    def writeListBegin(self, elemType, size):
        self.writeJSONArrayStart()
        self.context.writeNewLine(self.trans)

    def writeListEnd(self):
        self.writeJSONArrayEnd()

    def writeSetBegin(self, elemType, size):
        self.writeJSONArrayStart()
        self.context.writeNewLine(self.trans)

    def writeSetEnd(self):
        self.writeJSONArrayEnd()

    def writeBool(self, val):
        self.writeJSONBool(val)

    def writeByte(self, byte):
        self.writeJSONInteger(byte)

    def writeI16(self, i16):
        self.writeJSONInteger(i16)

    def writeI32(self, i32):
        self.writeJSONInteger(i32)

    def writeI64(self, i64):
        self.writeJSONInteger(i64)

    def writeDouble(self, d):
        self.writeJSONDouble(d)

    def writeFloat(self, f):
        self.writeJSONDouble(f)

    def writeString(self, outStr):
        self.writeJSONString(outStr)

    def writeBinary(self, outStr):
        self.writeJSONBase64(outStr)

    def readMessageBegin(self):
        self.readJSONArrayStart()
        self.skipWhitespace()
        if self.readJSONInteger() != THRIFT_VERSION_1:
            raise TProtocolException(
                TProtocolException.BAD_VERSION, "Message contained bad version."
            )
        name = self.readJSONString()
        mtype = self.readJSONInteger()
        seqid = self.readJSONInteger()

        return (name, mtype, seqid)

    def readMessageEnd(self):
        self.readJSONArrayEnd()

    def readStructBegin(self):
        self.readJSONObjectStart()
        # This is needed because of the very first call
        if self.spec.nextSpec is not None:
            self.pushSpec(self.spec.getNextSpec())

    def readStructEnd(self):
        self.readJSONObjectEnd()
        self.popSpec()

    def readFieldBegin(self):
        self.skipWhitespace()
        ch = self.reader.peek()
        if ch == JSON_OBJECT_END:
            return (None, TType.STOP, 0)
        self.context.read(self.reader)
        self.popContext()
        self.pushContext(
            self.pair_context_class(protocol=self, indentLevel=len(self.contexts))
        )
        self.skipWhitespace()
        fname = self.readJSONString()
        self.skipWhitespace()
        self.readJSONSyntaxChar(JSON_PAIR_SEPARATOR)
        self.context.skipColon = True
        self.skipWhitespace()
        if self.reader.peek() == b"n":
            for i in range(4):
                if self.reader.read() != b"null"[i : i + 1]:
                    raise TProtocolException(
                        TProtocolException.INVALID_DATA,
                        "Bad data encountered in null",
                    )

            self.context.read(self.reader)  # "consume" the colon we skipped
            return self.readFieldBegin()

        assert isinstance(self.spec, StructSpec)
        return self.spec.readFieldBegin(fname, self.guessTypeIdFromFirstByte)

    def readFieldEnd(self):
        return

    def readFieldStop(self):
        return

    def readNumber(self):
        return self.readJSONInteger()

    readByte = readNumber
    readI16 = readNumber
    readI32 = readNumber
    readI64 = readNumber

    def readDouble(self):
        return self.readJSONDouble()

    def readFloat(self):
        return self.readJSONDouble()

    def readString(self):
        return self.readJSONString()

    def readBinary(self):
        return self.readJSONBase64()

    def readBool(self):
        return self.readJSONBool()

    def readMapBegin(self):
        self.readJSONMapStart()
        self.skipWhitespace()
        self.pushContext(
            self.pair_context_class(
                protocol=self, indentLevel=len(self.contexts) - 1, isMapPair=True
            )
        )
        self.pushSpec(self.spec.getNextSpec())
        return self.spec.readMapBegin()

    def readMapEnd(self):
        self.popContext()
        self.readJSONMapEnd()
        self.popSpec()

    def peekMap(self):
        self.skipWhitespace()
        return self.reader.peek() != JSON_OBJECT_END

    def peekList(self):
        self.skipWhitespace()
        return self.reader.peek() != JSON_ARRAY_END

    peekSet = peekList

    def readListBegin(self):
        self.skipWhitespace()
        self.readJSONArrayStart()
        self.pushSpec(self.spec.getNextSpec())
        return self.spec.readListBegin()

    readSetBegin = readListBegin

    def readListEnd(self):
        self.skipWhitespace()
        self.readJSONArrayEnd()
        self.popSpec()

    readSetEnd = readListEnd


class TSimpleJSONProtocolFactory:
    def getProtocol(self, trans, spec=None):
        prot = TSimpleJSONProtocol(trans, spec)
        return prot

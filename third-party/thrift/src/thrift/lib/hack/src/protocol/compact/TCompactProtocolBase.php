<?hh
/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Compact implementation of the Thrift protocol.
 *
 */
abstract class TCompactProtocolBase extends TProtocol {

  const COMPACT_STOP = 0x00;
  const COMPACT_TRUE = 0x01;
  const COMPACT_FALSE = 0x02;
  const COMPACT_BYTE = 0x03;
  const COMPACT_I16 = 0x04;
  const COMPACT_I32 = 0x05;
  const COMPACT_I64 = 0x06;
  const COMPACT_DOUBLE = 0x07;
  const COMPACT_BINARY = 0x08;
  const COMPACT_LIST = 0x09;
  const COMPACT_SET = 0x0A;
  const COMPACT_MAP = 0x0B;
  const COMPACT_STRUCT = 0x0C;
  const COMPACT_FLOAT = 0x0D;

  const STATE_CLEAR = 0;
  const STATE_FIELD_WRITE = 1;
  const STATE_VALUE_WRITE = 2;
  const STATE_CONTAINER_WRITE = 3;
  const STATE_BOOL_WRITE = 4;
  const STATE_FIELD_READ = 5;
  const STATE_CONTAINER_READ = 6;
  const STATE_VALUE_READ = 7;
  const STATE_BOOL_READ = 8;

  const VERSION_MASK = 0x1f;
  const VERSION = 2;
  const VERSION_LOW = 1;
  const VERSION_DOUBLE_BE = 2;
  const PROTOCOL_ID = 0x82;
  const TYPE_MASK = 0xe0;
  const TYPE_SHIFT_AMOUNT = 5;

  const I16_MIN = -(1 << 15);
  const I16_MAX = (1 << 15) - 1;
  const I32_MIN = -(1 << 31);
  const I32_MAX = (1 << 31) - 1;

  protected static
    $ctypes = darray[
      TType::STOP => self::COMPACT_STOP,
      TType::BOOL => self::COMPACT_TRUE, // used for collection
      TType::BYTE => self::COMPACT_BYTE,
      TType::I16 => self::COMPACT_I16,
      TType::I32 => self::COMPACT_I32,
      TType::I64 => self::COMPACT_I64,
      TType::DOUBLE => self::COMPACT_DOUBLE,
      TType::FLOAT => self::COMPACT_FLOAT,
      TType::STRING => self::COMPACT_BINARY,
      TType::STRUCT => self::COMPACT_STRUCT,
      TType::LST => self::COMPACT_LIST,
      TType::SET => self::COMPACT_SET,
      TType::MAP => self::COMPACT_MAP,
    ];

  protected static
    $ttypes = darray[
      self::COMPACT_STOP => TType::STOP,
      self::COMPACT_TRUE => TType::BOOL, // used for collection
      self::COMPACT_FALSE => TType::BOOL,
      self::COMPACT_BYTE => TType::BYTE,
      self::COMPACT_I16 => TType::I16,
      self::COMPACT_I32 => TType::I32,
      self::COMPACT_I64 => TType::I64,
      self::COMPACT_DOUBLE => TType::DOUBLE,
      self::COMPACT_FLOAT => TType::FLOAT,
      self::COMPACT_BINARY => TType::STRING,
      self::COMPACT_STRUCT => TType::STRUCT,
      self::COMPACT_LIST => TType::LST,
      self::COMPACT_SET => TType::SET,
      self::COMPACT_MAP => TType::MAP,
    ];

  protected $state = self::STATE_CLEAR;
  protected $lastFid = 0;
  protected $boolFid = null;
  protected $boolValue = null;
  protected $structs = varray[];
  protected $containers = varray[];
  protected $version = self::VERSION;

  public function setWriteVersion($ver) {
    $this->version = $ver;
  }

  public function checkRange($value, $min, $max) {
    if ($value < $min || $value > $max) {
      throw new TProtocolException("Value is out of range");
    }
  }

  // Some varint / zigzag helper methods
  public function toZigZag($n, $bits) {
    return ($n << 1) ^ ($n >> ($bits - 1));
  }

  public function fromZigZag($n) {
    return ($n >> 1) ^ -($n & 1);
  }

  public function getVarint($data) {
    $out = "";
    while (true) {
      if (($data & ~0x7f) === 0) {
        $out .= chr($data);
        break;
      } else {
        $out .= chr(($data & 0xff) | 0x80);
        $data = $data >> 7;
      }
    }
    return $out;
  }

  public function writeVarint($data) {
    $out = $this->getVarint($data);
    $this->trans_->write($out);
    return strlen($out);
  }

  public function readVarint(&$result) {
    $idx = 0;
    $shift = 0;
    $result = 0;
    while (true) {
      $x = $this->trans_->readAll(1);
      $byte = ord($x);
      $idx += 1;
      $result |= ($byte & 0x7f) << $shift;
      if (($byte >> 7) === 0) {
        return $idx;
      }
      $shift += 7;
    }

    return $idx;
  }

  public function __construct($trans) {
    parent::__construct($trans);
  }

  <<__Override>>
  public function writeMessageBegin($name, $type, $seqid) {
    $written =
      $this->writeUByte(self::PROTOCOL_ID) +
      $this->writeUByte(
        $this->version | ($type << self::TYPE_SHIFT_AMOUNT),
      ) +
      $this->writeVarint($seqid) +
      $this->writeString($name);
    $this->state = self::STATE_VALUE_WRITE;
    return $written;
  }

  <<__Override>>
  public function writeMessageEnd() {
    $this->state = self::STATE_CLEAR;
    return 0;
  }

  <<__Override>>
  public function writeStructBegin($name) {
    $this->structs[] = varray[$this->state, $this->lastFid];
    $this->state = self::STATE_FIELD_WRITE;
    $this->lastFid = 0;
    return 0;
  }

  <<__Override>>
  public function writeStructEnd() {
    $old_values = array_pop(&$this->structs);
    $this->state = $old_values[0];
    $this->lastFid = $old_values[1];
    return 0;
  }

  <<__Override>>
  public function writeFieldStop() {
    return $this->writeByte(0);
  }

  public function writeFieldHeader($type, $fid) {
    $written = 0;
    $delta = $fid - $this->lastFid;
    if (0 < $delta && $delta <= 15) {
      $written = $this->writeUByte(($delta << 4) | $type);
    } else {
      $written = $this->writeByte($type) + $this->writeI16($fid);
    }
    $this->lastFid = $fid;
    return $written;
  }

  <<__Override>>
  public function writeFieldBegin($field_name, $field_type, $field_id) {
    if ($field_type == TType::BOOL) {
      $this->state = self::STATE_BOOL_WRITE;
      $this->boolFid = $field_id;
      return 0;
    } else {
      $this->state = self::STATE_VALUE_WRITE;
      return $this->writeFieldHeader(self::$ctypes[$field_type], $field_id);
    }
  }

  <<__Override>>
  public function writeFieldEnd() {
    $this->state = self::STATE_FIELD_WRITE;
    return 0;
  }

  public function writeCollectionBegin($etype, $size) {
    $written = 0;
    if ($size <= 14) {
      $written = $this->writeUByte($size << 4 | self::$ctypes[$etype]);
    } else {
      $written =
        $this->writeUByte(0xf0 | self::$ctypes[$etype]) +
        $this->writeVarint($size);
    }
    $this->containers[] = $this->state;
    $this->state = self::STATE_CONTAINER_WRITE;

    return $written;
  }

  <<__Override>>
  public function writeMapBegin($key_type, $val_type, $size) {
    $written = 0;
    if ($size == 0) {
      $written = $this->writeByte(0);
    } else {
      $written =
        $this->writeVarint($size) +
        $this->writeUByte(
          self::$ctypes[$key_type] << 4 | self::$ctypes[$val_type],
        );
    }
    $this->containers[] = $this->state;
    $this->state = self::STATE_CONTAINER_WRITE;
    return $written;
  }

  public function writeCollectionEnd() {
    $this->state = array_pop(&$this->containers);
    return 0;
  }

  <<__Override>>
  public function writeMapEnd() {
    return $this->writeCollectionEnd();
  }

  <<__Override>>
  public function writeListBegin($elem_type, $size) {
    return $this->writeCollectionBegin($elem_type, $size);
  }

  <<__Override>>
  public function writeListEnd() {
    return $this->writeCollectionEnd();
  }

  <<__Override>>
  public function writeSetBegin($elem_type, $size) {
    return $this->writeCollectionBegin($elem_type, $size);
  }

  <<__Override>>
  public function writeSetEnd() {
    return $this->writeCollectionEnd();
  }

  <<__Override>>
  public function writeBool($value) {
    if ($this->state == self::STATE_BOOL_WRITE) {
      $ctype = self::COMPACT_FALSE;
      if ($value) {
        $ctype = self::COMPACT_TRUE;
      }
      return $this->writeFieldHeader($ctype, $this->boolFid);
    } else if ($this->state == self::STATE_CONTAINER_WRITE) {
      return $this->writeByte(
        $value
          ? self::COMPACT_TRUE
          : self::COMPACT_FALSE,
      );
    } else {
      throw new TProtocolException('Invalid state in compact protocol');
    }
  }

  <<__Override>>
  public function writeByte($value) {
    $this->trans_->write(chr($value));
    return 1;
  }

  public function writeUByte($byte) {
    $this->trans_->write(chr($byte));
    return 1;
  }

  <<__Override>>
  public function writeI16($value) {
    $this->checkRange($value, self::I16_MIN, self::I16_MAX);
    $thing = $this->toZigZag($value, 16);
    return $this->writeVarint($thing);
  }

  <<__Override>>
  public function writeI32($value) {
    $this->checkRange($value, self::I32_MIN, self::I32_MAX);
    $thing = $this->toZigZag($value, 32);
    return $this->writeVarint($thing);
  }

  <<__Override>>
  public function writeDouble($value) {
    $data = pack('d', $value);
    if ($this->version >= self::VERSION_DOUBLE_BE) {
      $data = strrev($data);
    }
    $this->trans_->write($data);
    return 8;
  }

  <<__Override>>
  public function writeFloat($value) {
    $data = pack('f', $value);
    $data = strrev($data);
    $this->trans_->write($data);
    return 4;
  }

  <<__Override>>
  public function writeString($value) {
    $value = (string) $value;
    $len = strlen($value);
    $result = $this->writeVarint($len);
    if ($len) {
      $this->trans_->write($value);
    }
    return $result + $len;
  }

  <<__Override>>
  public function readFieldBegin(
    inout $name,
    inout $field_type,
    inout $field_id,
  ) {
    $result = $this->readUByte(&$field_type);
    $delta = $field_type >> 4;
    $field_type = $field_type & 0x0f;

    if ($field_type == self::COMPACT_STOP) {
      $field_id = 0;
      $field_type = $this->getTType($field_type);
      return $result;
    }

    if ($delta == 0) {
      $result += $this->readI16(&$field_id);
    } else {
      $field_id = $this->lastFid + $delta;
    }
    $this->lastFid = $field_id;

    if ($field_type == self::COMPACT_TRUE) {
      $this->state = self::STATE_BOOL_READ;
      $this->boolValue = true;
    } else if ($field_type == self::COMPACT_FALSE) {
      $this->state = self::STATE_BOOL_READ;
      $this->boolValue = false;
    } else {
      $this->state = self::STATE_VALUE_READ;
    }

    $field_type = $this->getTType($field_type);
    return $result;
  }

  <<__Override>>
  public function readFieldEnd() {
    $this->state = self::STATE_FIELD_READ;
    return 0;
  }

  public function readUByte(&$value) {
    $data = $this->trans_->readAll(1);
    $value = ord($data);
    return 1;
  }

  <<__Override>>
  public function readByte(&$value) {
    $data = $this->trans_->readAll(1);
    $value = ord($data);
    if ($value > 0x7f) {
      $value = 0 - (($value - 1) ^ 0xff);
    }
    return 1;
  }

  public function readZigZag(&$value) {
    $result = $this->readVarint(&$value);
    $value = $this->fromZigZag($value);
    return $result;
  }

  <<__Override>>
  public function readMessageBegin(inout $name, inout $type, inout $seqid) {
    $protoId = 0;
    $result = $this->readUByte(&$protoId);
    if ($protoId != self::PROTOCOL_ID) {
      throw new TProtocolException('Bad protocol id in TCompact message');
    }
    $verType = 0;
    $result += $this->readUByte(&$verType);
    $type =
      ($verType & self::TYPE_MASK) >>
      self::TYPE_SHIFT_AMOUNT;
    $this->version = $verType & self::VERSION_MASK;
    if (!($this->version <= self::VERSION &&
          $this->version >= self::VERSION_LOW)) {
      throw new TProtocolException('Bad version in TCompact message');
    }
    $result += $this->readVarint(&$seqid);
    $result += $this->readString(&$name);

    return $result;
  }

  <<__Override>>
  public function readMessageEnd() {
    return 0;
  }

  <<__Override>>
  public function readStructBegin(inout $name) {
    $name = ''; // unused
    $this->structs[] = varray[$this->state, $this->lastFid];
    $this->state = self::STATE_FIELD_READ;
    $this->lastFid = 0;
    return 0;
  }

  <<__Override>>
  public function readStructEnd() {
    $last = array_pop(&$this->structs);
    $this->state = $last[0];
    $this->lastFid = $last[1];
    return 0;
  }

  public function readCollectionBegin(&$type, &$size) {
    $sizeType = 0;
    $result = $this->readUByte(&$sizeType);
    $size = $sizeType >> 4;
    $type = $this->getTType($sizeType);
    if ($size == 15) {
      $result += $this->readVarint(&$size);
    }
    $this->containers[] = $this->state;
    $this->state = self::STATE_CONTAINER_READ;

    return $result;
  }

  <<__Override>>
  public function readMapBegin(inout $key_type, inout $val_type, inout $size) {
    $result = $this->readVarint(&$size);
    $types = 0;
    if ($size > 0) {
      $result += $this->readUByte(&$types);
    }
    $val_type = $this->getTType($types);
    $key_type = $this->getTType($types >> 4);
    $this->containers[] = $this->state;
    $this->state = self::STATE_CONTAINER_READ;

    return $result;
  }

  public function readCollectionEnd() {
    $this->state = array_pop(&$this->containers);
    return 0;
  }

  <<__Override>>
  public function readMapEnd() {
    return $this->readCollectionEnd();
  }

  <<__Override>>
  public function readListBegin(&$elem_type, &$size) {
    return $this->readCollectionBegin(&$elem_type, &$size);
  }

  <<__Override>>
  public function readListEnd() {
    return $this->readCollectionEnd();
  }

  <<__Override>>
  public function readSetBegin(&$elem_type, &$size) {
    return $this->readCollectionBegin(&$elem_type, &$size);
  }

  <<__Override>>
  public function readSetEnd() {
    return $this->readCollectionEnd();
  }

  <<__Override>>
  public function readBool(&$value) {
    if ($this->state == self::STATE_BOOL_READ) {
      $value = $this->boolValue;
      return 0;
    } else if ($this->state == self::STATE_CONTAINER_READ) {
      $result = $this->readByte(&$value);
      $value = $value == self::COMPACT_TRUE;
      return $result;
    } else {
      throw new TProtocolException('Invalid state in compact protocol');
    }
  }

  <<__Override>>
  public function readI16(&$value) {
    return $this->readZigZag(&$value);
  }

  <<__Override>>
  public function readI32(&$value) {
    return $this->readZigZag(&$value);
  }

  <<__Override>>
  public function readDouble(&$value) {
    $data = $this->trans_->readAll(8);
    if ($this->version >= self::VERSION_DOUBLE_BE) {
      $data = strrev($data);
    }
    $arr = unpack('d', $data);
    $value = $arr[1];
    return 8;
  }

  <<__Override>>
  public function readFloat(&$value) {
    $data = $this->trans_->readAll(4);
    $data = strrev($data);
    $arr = unpack('f', $data);
    $value = $arr[1];
    return 4;
  }

  <<__Override>>
  public function readString(&$value) {
    $len = 0;
    $result = $this->readVarint(&$len);
    if ($len) {
      $value = $this->trans_->readAll($len);
    } else {
      $value = '';
    }
    return $result + $len;
  }

  public function getTType($byte) {
    return self::$ttypes[$byte & 0x0f];
  }

  // If we are on a 32bit architecture we have to explicitly deal with
  // 64-bit twos-complement arithmetic since PHP wants to treat all ints
  // as signed and any int over 2^31 - 1 as a float

  // Read and write I64 as two 32 bit numbers $hi and $lo

  <<__Override>>
  public function readI64(&$value) {
    // Read varint from wire
    $hi = 0;
    $lo = 0;

    $idx = 0;
    $shift = 0;
    $arr = darray[];

    while (true) {
      $x = $this->trans_->readAll(1);
      $byte = ord($x);
      $idx += 1;
      if ($shift < 32) {
        $lo |= (($byte & 0x7f) << $shift) & 0x00000000ffffffff;
      }
      // Shift hi and lo together.
      if ($shift >= 32) {
        $hi |= (($byte & 0x7f) << ($shift - 32));
      } else if ($shift > 24) {
        $hi |= (($byte & 0x7f) >> ($shift - 24));
      }
      if (($byte >> 7) === 0) {
        break;
      }
      $shift += 7;
    }

    // Now, unzig it.
    $xorer = 0;
    if ($lo & 1) {
      $xorer = 0xffffffff;
    }
    $lo = ($lo >> 1) & 0x7fffffff;
    $lo = $lo | (($hi & 1) << 31);
    $hi = ($hi >> 1) ^ $xorer;
    $lo = $lo ^ $xorer;

    // Now put $hi and $lo back together
    if (true) {
      $isNeg = $hi < 0;

      // Check for a negative
      if ($isNeg) {
        $hi = ~$hi & (int) 0xffffffff;
        $lo = ~$lo & (int) 0xffffffff;

        if ($lo == (int) 0xffffffff) {
          $hi++;
          $lo = 0;
        } else {
          $lo++;
        }
      }

      // Force 32bit words in excess of 2G to be positive - we deal with sign
      // explicitly below

      if ($hi & (int) 0x80000000) {
        $hi &= (int) 0x7fffffff;
        $hi += 0x80000000;
      }

      if ($lo & (int) 0x80000000) {
        $lo &= (int) 0x7fffffff;
        $lo += 0x80000000;
      }

      $value = $hi * 4294967296 + $lo;

      if ($isNeg) {
        $value = 0 - $value;
      }
    } else {

      // Upcast negatives in LSB bit
      if ($arr[2] & 0x80000000) {
        $arr[2] = $arr[2] & 0xffffffff;
      }

      // Check for a negative
      if ($arr[1] & 0x80000000) {
        $arr[1] = $arr[1] & 0xffffffff;
        $arr[1] = $arr[1] ^ 0xffffffff;
        $arr[2] = $arr[2] ^ 0xffffffff;
        $value = 0 - $arr[1] * 4294967296 - $arr[2] - 1;
      } else {
        $value = $arr[1] * 4294967296 + $arr[2];
      }
    }

    return $idx;
  }

  <<__Override>>
  public function writeI64($value) {
    // If we are in an I32 range, use the easy method below.
    if (($value > 4294967296) || ($value < -4294967296)) {
      // Convert $value to $hi and $lo
      $neg = $value < 0;

      if ($neg) {
        $value *= -1;
      }

      $hi = (int) $value >> 32;
      $lo = (int) $value & 0xffffffff;

      if ($neg) {
        $hi = ~$hi;
        $lo = ~$lo;
        if (($lo & (int) 0xffffffff) == (int) 0xffffffff) {
          $lo = 0;
          $hi++;
        } else {
          $lo++;
        }
      }

      // Now do the zigging and zagging.
      $xorer = 0;
      if ($neg) {
        $xorer = 0xffffffff;
      }
      $lowbit = ($lo >> 31) & 1;
      $hi = ($hi << 1) | $lowbit;
      $lo = ($lo << 1);
      $lo = ($lo ^ $xorer) & 0xffffffff;
      $hi = ($hi ^ $xorer) & 0xffffffff;

      // now write out the varint, ensuring we shift both hi and lo
      $out = "";
      while (true) {
        if (($lo & ~0x7f) === 0 && $hi === 0) {
          $out .= chr($lo);
          break;
        } else {
          $out .= chr(($lo & 0xff) | 0x80);
          $lo = $lo >> 7;
          $lo = $lo | ($hi << 25);
          $hi = $hi >> 7;
          // Right shift carries sign, but we don't want it to.
          $hi = $hi & (127 << 25);
        }
      }

      $this->trans_->write($out);

      return strlen($out);
    } else {
      return $this->writeVarint($this->toZigZag($value, 64));
    }
  }
}

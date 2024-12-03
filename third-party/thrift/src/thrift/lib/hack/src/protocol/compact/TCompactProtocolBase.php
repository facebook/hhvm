<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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
 *
 */

// @oss-enable: use namespace FlibSL\{C, Math, Str, Vec};

/**
 * Compact implementation of the Thrift protocol.
 *
 */
<<Oncalls('thrift')>> // @oss-disable
abstract class TCompactProtocolBase extends TProtocol {
  const ctx CReadWriteDefault = [zoned_shallow];

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

  const int I16_MIN = -(1 << 15);
  const int I16_MAX = (1 << 15) - 1;
  const int I32_MIN = -(1 << 31);
  const int I32_MAX = (1 << 31) - 1;

  const dict<TType, int> CTYPES = dict[
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

  const dict<int, TType> TTYPES = dict[
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

  protected int $state = self::STATE_CLEAR;
  protected int $lastFid = 0;
  protected ?int $boolFid = null;
  protected ?bool $boolValue = null;
  protected vec<vec<int>> $structs = HH\array_mark_legacy(vec[]);
  protected vec<int> $containers = HH\array_mark_legacy(vec[]);
  protected int $version = self::VERSION;

  public function setWriteVersion(int $ver)[write_props]: void {
    $this->version = $ver;
  }

  public function checkRange(int $value, int $min, int $max)[]: void {
    if ($value < $min || $value > $max) {
      throw new TProtocolException("Value is out of range");
    }
  }

  // Some varint / zigzag helper methods
  public function toZigZag(int $n, int $bits)[]: int {
    return ($n << 1) ^ ($n >> ($bits - 1));
  }

  public function fromZigZag(int $n)[]: int {
    return ($n >> 1) ^ -($n & 1);
  }

  public function getVarint(int $data)[]: string {
    $out = "";
    while (true) {
      if (($data & ~0x7f) === 0) {
        $out .= PHP\chr($data);
        break;
      } else {
        $out .= PHP\chr(($data & 0xff) | 0x80);
        $data = $data >> 7;
      }
    }
    return $out;
  }

  public function writeVarint(int $data)[zoned_shallow]: int {
    $out = $this->getVarint($data);
    $this->trans_->write($out);
    return Str\length($out);
  }

  public function readVarint(inout int $result)[zoned_shallow]: int {
    $idx = 0;
    $shift = 0;
    $result = 0;
    while (true) {
      $x = $this->trans_->readAll(1);
      $byte = PHP\ord($x);
      $idx += 1;
      $result |= ($byte & 0x7f) << $shift;
      if (($byte >> 7) === 0) {
        return $idx;
      }
      $shift += 7;
    }
  }

  public function __construct(TTransport $trans)[] {
    parent::__construct($trans);
  }

  <<__Override>>
  public function writeMessageBegin(
    string $name,
    TMessageType $type,
    int $seqid,
  )[zoned_shallow]: int {
    $written = $this->writeUByte(self::PROTOCOL_ID) +
      $this->writeUByte($this->version | ($type << self::TYPE_SHIFT_AMOUNT)) +
      $this->writeVarint($seqid) +
      $this->writeString($name);
    $this->state = self::STATE_VALUE_WRITE;
    return $written;
  }

  <<__Override>>
  public function writeMessageEnd()[write_props]: int {
    $this->state = self::STATE_CLEAR;
    return 0;
  }

  <<__Override>>
  public function writeStructBegin(string $_name)[write_props]: int {
    $this->structs[] = vec[$this->state, $this->lastFid];
    $this->state = self::STATE_FIELD_WRITE;
    $this->lastFid = 0;
    return 0;
  }

  <<__Override>>
  public function writeStructEnd()[write_props]: int {
    $__structs = $this->structs;
    // Assign empty value into property to avoid
    // triggering copy-on-write
    $this->structs = vec[];
    $old_values = PHP\array_pop(inout $__structs);
    $this->structs = $__structs;
    $this->state = HH\FIXME\UNSAFE_NONNULL_CAST<vec<int>>(
      $old_values,
      'FIXME[4063] Exposed by typing PHP\array_pop',
    )[0];
    $this->lastFid = HH\FIXME\UNSAFE_NONNULL_CAST<vec<int>>(
      $old_values,
      'FIXME[4063] Exposed by typing PHP\array_pop',
    )[1];
    return 0;
  }

  <<__Override>>
  public function writeFieldStop()[zoned_shallow]: int {
    return $this->writeByte(0);
  }

  public function writeFieldHeader(int $type, int $fid)[zoned_shallow]: int {
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
  public function writeFieldBegin(
    string $_field_name,
    TType $field_type,
    int $field_id,
  )[zoned_shallow]: int {
    if ($field_type === TType::BOOL) {
      $this->state = self::STATE_BOOL_WRITE;
      $this->boolFid = $field_id;
      return 0;
    } else {
      $this->state = self::STATE_VALUE_WRITE;
      return $this->writeFieldHeader(self::CTYPES[$field_type], $field_id);
    }
  }

  <<__Override>>
  public function writeFieldEnd()[write_props]: int {
    $this->state = self::STATE_FIELD_WRITE;
    return 0;
  }

  public function writeCollectionBegin(
    TType $etype,
    int $size,
  )[zoned_shallow]: int {
    if ($size <= 14) {
      $written = $this->writeUByte($size << 4 | self::CTYPES[$etype]);
    } else {
      $written = $this->writeUByte(0xf0 | self::CTYPES[$etype]) +
        $this->writeVarint($size);
    }
    $this->containers[] = $this->state;
    $this->state = self::STATE_CONTAINER_WRITE;

    return $written;
  }

  <<__Override>>
  public function writeMapBegin(
    TType $key_type,
    TType $val_type,
    int $size,
  )[zoned_shallow]: int {
    if ($size === 0) {
      $written = $this->writeByte(0);
    } else {
      $written = $this->writeVarint($size) +
        $this->writeUByte(
          self::CTYPES[$key_type] << 4 | self::CTYPES[$val_type],
        );
    }
    $this->containers[] = $this->state;
    $this->state = self::STATE_CONTAINER_WRITE;
    return $written;
  }

  public function writeCollectionEnd()[write_props]: int {
    $__containers = $this->containers;
    // Assign empty value into property to avoid
    // triggering copy-on-write
    $this->containers = vec[];
    $this->state = HH\FIXME\UNSAFE_NONNULL_CAST<int>(
      PHP\array_pop(inout $__containers),
      'FIXME[4110] Exposed by typing PHP\array_pop',
    );
    $this->containers = $__containers;
    return 0;
  }

  <<__Override>>
  public function writeMapEnd()[write_props]: int {
    return $this->writeCollectionEnd();
  }

  <<__Override>>
  public function writeListBegin(
    TType $elem_type,
    int $size,
  )[zoned_shallow]: int {
    return $this->writeCollectionBegin($elem_type, $size);
  }

  <<__Override>>
  public function writeListEnd()[write_props]: int {
    return $this->writeCollectionEnd();
  }

  <<__Override>>
  public function writeSetBegin(
    TType $elem_type,
    int $size,
  )[zoned_shallow]: int {
    return $this->writeCollectionBegin($elem_type, $size);
  }

  <<__Override>>
  public function writeSetEnd()[write_props]: int {
    return $this->writeCollectionEnd();
  }

  <<__Override>>
  public function writeBool(bool $value)[zoned_shallow]: int {
    if ($this->state === self::STATE_BOOL_WRITE) {
      $ctype = self::COMPACT_FALSE;
      if ($value) {
        $ctype = self::COMPACT_TRUE;
      }
      // This worked before with a strict type in writeFieldHeader
      // TODO (partisan): Rewrite to avoid null
      return $this->writeFieldHeader($ctype, (int)$this->boolFid);
    } else if ($this->state === self::STATE_CONTAINER_WRITE) {
      return
        $this->writeByte($value ? self::COMPACT_TRUE : self::COMPACT_FALSE);
    } else {
      throw new TProtocolException('Invalid state in compact protocol');
    }
  }

  <<__Override>>
  public function writeByte(int $value)[zoned_shallow]: int {
    $this->trans_->write(PHP\chr($value));
    return 1;
  }

  public function writeUByte(int $byte)[zoned_shallow]: int {
    $this->trans_->write(PHP\chr($byte));
    return 1;
  }

  <<__Override>>
  public function writeI16(int $value)[zoned_shallow]: int {
    $this->checkRange($value, self::I16_MIN, self::I16_MAX);
    $thing = $this->toZigZag($value, 16);
    return $this->writeVarint($thing);
  }

  <<__Override>>
  public function writeI32(int $value)[zoned_shallow]: int {
    $this->checkRange($value, self::I32_MIN, self::I32_MAX);
    $thing = $this->toZigZag($value, 32);
    return $this->writeVarint($thing);
  }

  <<__Override>>
  public function writeDouble(float $value)[zoned_shallow]: int {
    $data = PHP\pack('d', $value);
    if ($this->version >= self::VERSION_DOUBLE_BE) {
      $data = PHP\strrev($data);
    }
    $this->trans_->write($data);
    return 8;
  }

  <<__Override>>
  public function writeFloat(float $value)[zoned_shallow]: int {
    $data = PHP\pack('f', $value);
    $data = Str\reverse($data);
    $this->trans_->write($data);
    return 4;
  }

  <<__Override>>
  public function writeString(string $value)[zoned_shallow]: int {
    $value = (string)$value;
    $len = Str\length($value);
    $result = $this->writeVarint($len);
    if ($len) {
      $this->trans_->write($value);
    }
    return $result + $len;
  }

  <<__Override>>
  public function readFieldBegin(
    inout ?string $name,
    inout ?TType $field_type,
    inout ?int $field_id,
  )[zoned_shallow]: int {
    $field_type_ubyte = 0;
    $result = $this->readUByte(inout $field_type_ubyte);
    $delta = $field_type_ubyte >> 4;
    $field_type = $field_type_ubyte & 0x0f;

    if ($field_type === self::COMPACT_STOP) {
      $field_id = 0;
      $field_type = $this->getTType($field_type_ubyte);
      return $result;
    }

    if ($delta === 0) {
      // TODO (partisan): Remove this after refactoring read methods to not use
      // inout.
      $field_id_nonnull = 0;
      $result += $this->readI16(inout $field_id_nonnull);
      $field_id = $field_id_nonnull;
    } else {
      $field_id = $this->lastFid + $delta;
    }
    $this->lastFid = $field_id;

    if ($field_type === self::COMPACT_TRUE) {
      $this->state = self::STATE_BOOL_READ;
      $this->boolValue = true;
    } else if ($field_type === self::COMPACT_FALSE) {
      $this->state = self::STATE_BOOL_READ;
      $this->boolValue = false;
    } else {
      $this->state = self::STATE_VALUE_READ;
    }

    $field_type = $this->getTType($field_type_ubyte);
    return $result;
  }

  <<__Override>>
  public function readFieldEnd()[write_props]: int {
    $this->state = self::STATE_FIELD_READ;
    return 0;
  }

  public function readUByte(inout int $value)[zoned_shallow]: int {
    $data = $this->trans_->readAll(1);
    $value = PHP\ord($data);
    return 1;
  }

  <<__Override>>
  public function readByte(inout int $value)[zoned_shallow]: int {
    $data = $this->trans_->readAll(1);
    $value = PHP\ord($data);
    if ($value > 0x7f) {
      $value = 0 - (($value - 1) ^ 0xff);
    }
    return 1;
  }

  public function readZigZag(inout int $value)[zoned_shallow]: int {
    $result = $this->readVarint(inout $value);
    $value = $this->fromZigZag($value);
    return $result;
  }

  <<__Override>>
  public function readMessageBegin(
    inout string $name,
    inout int $type,
    inout int $seqid,
  )[zoned_shallow]: int {
    $protoId = 0;
    $result = $this->readUByte(inout $protoId);
    if ($protoId !== self::PROTOCOL_ID) {
      throw new TProtocolException('Bad protocol id in TCompact message');
    }
    $verType = 0;
    $result += $this->readUByte(inout $verType);
    $type = ($verType & self::TYPE_MASK) >> self::TYPE_SHIFT_AMOUNT;
    $this->version = $verType & self::VERSION_MASK;
    if (
      !($this->version <= self::VERSION && $this->version >= self::VERSION_LOW)
    ) {
      throw new TProtocolException('Bad version in TCompact message');
    }
    $result += $this->readVarint(inout $seqid);
    $result += $this->readString(inout $name);

    return $result;
  }

  <<__Override>>
  public function readMessageEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function readStructBegin(inout ?string $name)[write_props]: int {
    $name = ''; // unused
    $this->structs[] = vec[$this->state, $this->lastFid];
    $this->state = self::STATE_FIELD_READ;
    $this->lastFid = 0;
    return 0;
  }

  <<__Override>>
  public function readStructEnd()[write_props]: int {
    $__structs = $this->structs;
    // Assign empty value into property to avoid
    // triggering copy-on-write
    $this->structs = vec[];
    $last = PHP\array_pop(inout $__structs);
    $this->structs = $__structs;
    $this->state = HH\FIXME\UNSAFE_NONNULL_CAST<vec<int>>(
      $last,
      'FIXME[4063] Exposed by typing PHP\array_pop',
    )[0];
    $this->lastFid = HH\FIXME\UNSAFE_NONNULL_CAST<vec<int>>(
      $last,
      'FIXME[4063] Exposed by typing PHP\array_pop',
    )[1];
    return 0;
  }

  public function readCollectionBegin(
    inout ?TType $type,
    inout ?int $size,
  )[zoned_shallow]: int {
    $sizeType = 0;
    $result = $this->readUByte(inout $sizeType);
    $size = $sizeType >> 4;
    $type = $this->getTType($sizeType);
    if ($size === 15) {
      $result += $this->readVarint(inout $size);
    }
    $this->containers[] = $this->state;
    $this->state = self::STATE_CONTAINER_READ;

    return $result;
  }

  <<__Override>>
  public function readMapBegin(
    inout ?TType $key_type,
    inout ?TType $val_type,
    inout ?int $size,
  )[zoned_shallow]: int {
    // TODO (partisan): Remove after refactoring of read methods to not use
    // inout.
    $size_nonnull = -1;
    $result = $this->readVarint(inout $size_nonnull);
    $size = $size_nonnull;
    $types = 0;
    if ($size > 0) {
      $result += $this->readUByte(inout $types);
    }
    $val_type = $this->getTType($types);
    $key_type = $this->getTType($types >> 4);
    $this->containers[] = $this->state;
    $this->state = self::STATE_CONTAINER_READ;

    return $result;
  }

  public function readCollectionEnd()[write_props]: int {
    $__containers = $this->containers;
    // Assign empty value into property to avoid
    // triggering copy-on-write
    $this->containers = vec[];
    $this->state = HH\FIXME\UNSAFE_NONNULL_CAST<int>(
      PHP\array_pop(inout $__containers),
      'FIXME[4110] Exposed by typing PHP\array_pop',
    );
    $this->containers = $__containers;
    return 0;
  }

  <<__Override>>
  public function readMapEnd()[write_props]: int {
    return $this->readCollectionEnd();
  }

  <<__Override>>
  public function readListBegin(
    inout ?TType $elem_type,
    inout ?int $size,
  )[zoned_shallow]: int {
    // TODO (partisan): Remove after refactoring of read methods to not use
    // inout.
    $size_nonnull = -1;
    $result = $this->readCollectionBegin(inout $elem_type, inout $size_nonnull);
    $size = $size_nonnull;
    return $result;
  }

  <<__Override>>
  public function readListEnd()[write_props]: int {
    return $this->readCollectionEnd();
  }

  <<__Override>>
  public function readSetBegin(
    inout ?TType $elem_type,
    inout ?int $size,
  )[zoned_shallow]: int {
    // TODO (partisan): Remove after refactoring of read methods to not use
    // inout.
    $size_nonnull = -1;
    $result = $this->readCollectionBegin(inout $elem_type, inout $size_nonnull);
    $size = $size_nonnull;
    return $result;
  }

  <<__Override>>
  public function readSetEnd()[write_props]: int {
    return $this->readCollectionEnd();
  }

  <<__Override>>
  public function readBool(inout bool $value)[zoned_shallow]: int {
    if ($this->state === self::STATE_BOOL_READ) {
      // TODO (partisan): Refactor the code so this check is no longer
      // necessary.
      if ($this->boolValue is null) {
        throw new TProtocolException(
          '$this->boolValue shouldn\'t be null in the STATE_BOOL_READ.',
        );
      }
      $value = $this->boolValue;
      return 0;
    } else if ($this->state === self::STATE_CONTAINER_READ) {
      $value_byte = -1;
      $result = $this->readByte(inout $value_byte);
      $value = $value_byte === self::COMPACT_TRUE;
      return $result;
    } else {
      throw new TProtocolException('Invalid state in compact protocol');
    }
  }

  <<__Override>>
  public function readI16(inout int $value)[zoned_shallow]: int {
    return $this->readZigZag(inout $value);
  }

  <<__Override>>
  public function readI32(inout int $value)[zoned_shallow]: int {
    return $this->readZigZag(inout $value);
  }

  <<__Override>>
  public function readDouble(inout float $value)[zoned_shallow]: int {
    $data = $this->trans_->readAll(8);
    if ($this->version >= self::VERSION_DOUBLE_BE) {
      $data = PHP\strrev($data);
    }
    $arr = PHP\unpack('d', $data);
    $value = $arr[1];
    $value as float;
    return 8;
  }

  <<__Override>>
  public function readFloat(inout float $value)[zoned_shallow]: int {
    $data = $this->trans_->readAll(4);
    $data = PHP\strrev($data);
    $arr = PHP\unpack('f', $data);
    $value = $arr[1];
    $value as float;
    return 4;
  }

  <<__Override>>
  public function readString(inout string $value)[zoned_shallow]: int {
    $len = 0;
    $result = $this->readVarint(inout $len);
    if ($len) {
      $value = $this->trans_->readAll($len);
    } else {
      $value = '';
    }
    return $result + $len;
  }

  public function getTType(int $byte)[]: TType {
    return self::TTYPES[$byte & 0x0f];
  }

  // If we are on a 32bit architecture we have to explicitly deal with
  // 64-bit twos-complement arithmetic since PHP wants to treat all ints
  // as signed and any int over 2^31 - 1 as a float

  // Read and write I64 as two 32 bit numbers $hi and $lo

  <<__Override>>
  public function readI64(inout int $value)[zoned_shallow]: int {
    // Read varint from wire
    $hi = 0;
    $lo = 0;

    $idx = 0;
    $shift = 0;

    while (true) {
      $x = $this->trans_->readAll(1);
      $byte = PHP\ord($x);
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
    $is_neg = $hi < 0;

    // Check for a negative
    if ($is_neg) {
      $hi = ~$hi & (int)0xffffffff;
      $lo = ~$lo & (int)0xffffffff;

      if ($lo === (int)0xffffffff) {
        $hi++;
        $lo = 0;
      } else {
        $lo++;
      }
    }

    // Force 32bit words in excess of 2G to be positive - we deal with sign
    // explicitly below

    if ($hi & (int)0x80000000) {
      $hi &= (int)0x7fffffff;
      $hi += 0x80000000;
    }

    if ($lo & (int)0x80000000) {
      $lo &= (int)0x7fffffff;
      $lo += 0x80000000;
    }

    $value = $hi * 4294967296 + $lo;

    if ($is_neg) {
      $value = 0 - $value;
    }

    return $idx;
  }

  <<__Override>>
  public function writeI64(int $value)[zoned_shallow]: int {
    // If we are in an I32 range, use the easy method below.
    if (($value > 4294967296) || ($value < -4294967296)) {
      // Convert $value to $hi and $lo
      $neg = $value < 0;

      if ($neg) {
        $value *= -1;
      }

      $hi = (int)$value >> 32;
      $lo = (int)$value & 0xffffffff;

      if ($neg) {
        $hi = ~$hi;
        $lo = ~$lo;
        if (($lo & (int)0xffffffff) === (int)0xffffffff) {
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
          $out .= PHP\chr($lo);
          break;
        } else {
          $out .= PHP\chr(($lo & 0xff) | 0x80);
          $lo = $lo >> 7;
          $lo = $lo | ($hi << 25);
          $hi = $hi >> 7;
          // Right shift carries sign, but we don't want it to.
          $hi = $hi & (127 << 25);
        }
      }

      $this->trans_->write($out);

      return Str\length($out);
    } else {
      return $this->writeVarint($this->toZigZag($value, 64));
    }
  }
}

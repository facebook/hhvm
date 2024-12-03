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
 * Binary implementation of the Thrift protocol.
 */
<<Oncalls('thrift')>> // @oss-disable
abstract class TBinaryProtocolBase extends TProtocol {
  const ctx CReadWriteDefault = [zoned_shallow];

  const VERSION_MASK = 0xffff0000;
  const VERSION_1 = 0x80010000;
  const DEFAULT_MEMORY_LIMIT = 128000000; //128M, the default

  protected bool $littleEndian = false;
  protected int $memoryLimit = self::DEFAULT_MEMORY_LIMIT;
  protected ?int $sequenceID = null;

  public function __construct(
    TTransport $trans,
    protected bool $strictRead = false,
    protected bool $strictWrite = true,
  )[read_globals] {
    parent::__construct($trans);
    if (PHP\pack('S', 1) == "\x01\x00") {
      $this->littleEndian = true;
    }
    $this->memoryLimit = MemoryLimit::get() ?? self::DEFAULT_MEMORY_LIMIT;
  }

  <<__Override>>
  public function writeMessageBegin(
    string $name,
    TMessageType $type,
    int $seqid,
  )[zoned_shallow]: int {
    if ($this->strictWrite) {
      $version = self::VERSION_1 | $type;
      return $this->writeI32($version) +
        $this->writeString($name) +
        $this->writeI32($seqid);
    } else {
      return $this->writeString($name) +
        $this->writeByte($type) +
        $this->writeI32($seqid);
    }
  }

  <<__Override>>
  public function writeMessageEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function writeStructBegin(string $_name)[]: int {
    return 0;
  }

  <<__Override>>
  public function writeStructEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function writeFieldBegin(
    string $_field_name,
    TType $field_type,
    int $field_id,
  )[zoned_shallow]: int {
    return $this->writeByte($field_type) + $this->writeI16($field_id);
  }

  <<__Override>>
  public function writeFieldEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function writeFieldStop()[zoned_shallow]: int {
    return $this->writeByte(TType::STOP);
  }

  <<__Override>>
  public function writeMapBegin(
    TType $key_type,
    TType $val_type,
    int $size,
  )[zoned_shallow]: int {
    return $this->writeByte($key_type) +
      $this->writeByte($val_type) +
      $this->writeI32($size);
  }

  <<__Override>>
  public function writeMapEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function writeListBegin(
    TType $elem_type,
    int $size,
  )[zoned_shallow]: int {
    return $this->writeByte($elem_type) + $this->writeI32($size);
  }

  <<__Override>>
  public function writeListEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function writeSetBegin(
    TType $elem_type,
    int $size,
  )[zoned_shallow]: int {
    return $this->writeByte($elem_type) + $this->writeI32($size);
  }

  <<__Override>>
  public function writeSetEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function writeBool(bool $value)[zoned_shallow]: int {
    $data = PHP\pack('c', $value ? 1 : 0);
    $this->trans_->write($data);
    return 1;
  }

  <<__Override>>
  public function writeByte(int $value)[zoned_shallow]: int {
    $this->trans_->write(PHP\chr($value));
    return 1;
  }

  <<__Override>>
  public function writeI16(int $value)[zoned_shallow]: int {
    $data = PHP\chr($value).PHP\chr($value >> 8);
    if ($this->littleEndian) {
      $data = PHP\strrev($data);
    }
    $this->trans_->write($data);
    return 2;
  }

  <<__Override>>
  public function writeI32(int $value)[zoned_shallow]: int {
    $data = PHP\chr($value).
      PHP\chr($value >> 8).
      PHP\chr($value >> 16).
      PHP\chr($value >> 24);
    if ($this->littleEndian) {
      $data = PHP\strrev($data);
    }
    $this->trans_->write($data);
    return 4;
  }

  <<__Override>>
  public function writeI64(?int $value)[zoned_shallow]: int {
    $value ??= 0;
    // If we are on a 32bit architecture we have to explicitly deal with
    // 64-bit twos-complement arithmetic since PHP wants to treat all ints
    // as signed and any int over 2^31 - 1 as a float
    if (PHP_INT_SIZE === 4) {
      $neg = $value < 0;

      if ($neg) {
        $value *= -1;
      }

      $hi = (int)($value / 4294967296);
      $lo = (int)$value;

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
      $data = PHP\pack('N2', $hi, $lo);
    } else {
      $data = PHP\chr($value).
        PHP\chr($value >> 8).
        PHP\chr($value >> 16).
        PHP\chr($value >> 24).
        PHP\chr($value >> 32).
        PHP\chr($value >> 40).
        PHP\chr($value >> 48).
        PHP\chr($value >> 56);
      if ($this->littleEndian) {
        $data = PHP\strrev($data);
      }
    }

    $this->trans_->write($data);
    return 8;
  }

  <<__Override>>
  public function writeDouble(float $value)[zoned_shallow]: int {
    $data = PHP\pack('d', $value);
    if ($this->littleEndian) {
      $data = PHP\strrev($data);
    }
    $this->trans_->write($data);
    return 8;
  }

  <<__Override>>
  public function writeFloat(float $value)[zoned_shallow]: int {
    $data = PHP\pack('f', $value);
    if ($this->littleEndian) {
      $data = PHP\strrev($data);
    }
    $this->trans_->write($data);
    return 4;
  }

  <<__Override>>
  public function writeString(string $value)[zoned_shallow]: int {
    $len = Str\length($value);
    $result = $this->writeI32($len);
    if ($len) {
      $this->trans_->write($value);
    }
    return $result + $len;
  }

  private function unpackI32(string $data)[]: int {
    $value = PHP\ord($data[3]) |
      (PHP\ord($data[2]) << 8) |
      (PHP\ord($data[1]) << 16) |
      (PHP\ord($data[0]) << 24);
    if ($value > 0x7fffffff) {
      $value = 0 - (($value - 1) ^ 0xffffffff);
    }
    return $value;
  }

  /**
   * Returns the sequence ID of the next message; only valid when called
   * before readMessageBegin()
   */
  <<__Deprecated(
    'This function was found unused by CodemodRuleDeprecateUnusedClassMethod',
  )>>
  public function peekSequenceID()[zoned_shallow]: int {
    $trans = $this->trans_;
    if (!($trans is IThriftBufferedTransport)) {
      throw new TProtocolException(
        get_class($this->trans_).' does not support peek',
        TProtocolException::BAD_VERSION,
      );
    }
    if ($this->sequenceID !== null) {
      throw new TProtocolException(
        'peekSequenceID can only be called '.'before readMessageBegin',
        TProtocolException::INVALID_DATA,
      );
    }

    $data = $trans->peek(4);
    $sz = $this->unpackI32($data);
    $start = 4;

    if ($sz < 0) {
      $version = $sz & self::VERSION_MASK;
      if ($version !== self::VERSION_1) {
        throw new TProtocolException(
          'Bad version identifier: '.$sz,
          TProtocolException::BAD_VERSION,
        );
      }
      // skip name string
      $data = $trans->peek(4, $start);
      $name_len = $this->unpackI32($data);
      $start += 4 + $name_len;
      // peek seqId
      $data = $trans->peek(4, $start);
      $seqid = $this->unpackI32($data);
    } else {
      if ($this->strictRead) {
        throw new TProtocolException(
          'No version identifier, old protocol client?',
          TProtocolException::BAD_VERSION,
        );
      } else {
        // need to guard the length from mis-configured other type of TCP server
        // for example, if mis-configure sshd, will read 'SSH-' as the length
        // if memory limit is -1, means no limit.
        if ($this->memoryLimit > 0 && $sz > $this->memoryLimit) {
          throw new TProtocolException(
            'Length overflow: '.$sz,
            TProtocolException::SIZE_LIMIT,
          );
        }
        // Handle pre-versioned input
        $start += $sz;
        // skip type byte
        $start += 1;
        // peek seqId
        $data = $trans->peek(4, $start);
        $seqid = $this->unpackI32($data);
      }
    }
    return $seqid;
  }

  <<__Override>>
  public function readMessageBegin(
    inout string $name,
    inout int $type,
    inout int $seqid,
  )[zoned_shallow]: int {
    $sz = 0;
    $result = $this->readI32(inout $sz);
    if ($sz < 0) {
      $version = $sz & self::VERSION_MASK;
      if ($version !== self::VERSION_1) {
        throw new TProtocolException(
          'Bad version identifier: '.$sz,
          TProtocolException::BAD_VERSION,
        );
      }
      $type = $sz & 0x000000ff;
      // TODO (partisan): Refactor this as a part of inout deprecation in read
      // methods.
      $name_nonnull = '';
      $name_res = $this->readString(inout $name_nonnull);
      $name = $name_nonnull;
      // TODO (partisan): Refactor this as a part of inout deprecation in read
      // methods.
      $seqid_nonnull = -1;
      $seqid_res = $this->readI32(inout $seqid_nonnull);
      $seqid = $seqid_nonnull;
      $result += $name_res + $seqid_res;
    } else {
      if ($this->strictRead) {
        throw new TProtocolException(
          'No version identifier, old protocol client?',
          TProtocolException::BAD_VERSION,
        );
      } else {
        // need to guard the length from mis-configured other type of TCP server
        // for example, if mis-configure sshd, will read 'SSH-' as the length
        // if memory limit is -1, means no limit.
        if ($this->memoryLimit > 0 && $sz > $this->memoryLimit) {
          throw new TProtocolException(
            'Length overflow: '.$sz,
            TProtocolException::SIZE_LIMIT,
          );
        }
        // Handle pre-versioned input
        $name = $this->trans_->readAll($sz);
        $type_byte = -1;
        $type_res = $this->readByte(inout $type_byte);
        $type = TMessageType::assert($type_byte);
        // TODO (partisan): Refactor this as a part of inout deprecation in read
        // methods.
        $seqid_nonnull = -1;
        $seqid_res = $this->readI32(inout $seqid_nonnull);
        $seqid = $seqid_nonnull;
        $result += $sz + $type_res + $seqid_res;
      }
    }
    $this->sequenceID = $seqid;
    return $result;
  }

  <<__Override>>
  public function readMessageEnd()[write_props]: int {
    $this->sequenceID = null;
    return 0;
  }

  <<__Override>>
  public function readStructBegin(inout ?string $name)[]: int {
    $name = '';
    return 0;
  }

  <<__Override>>
  public function readStructEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function readFieldBegin(
    inout ?string $_name,
    inout ?TType $field_type,
    inout ?int $field_id,
  )[zoned_shallow]: int {
    $field_type_byte = -1;
    $result = $this->readByte(inout $field_type_byte);
    $field_type = TType::assert($field_type_byte);
    if ($field_type === TType::STOP) {
      $field_id = 0;
      return $result;
    }
    // TODO (partisan): Refactor this as a part of inout deprecation in read
    // methods.
    $field_id_nonnull = -1;
    $fid_res = $this->readI16(inout $field_id_nonnull);
    $field_id = $field_id_nonnull;
    $result += $fid_res;
    return $result;
  }

  <<__Override>>
  public function readFieldEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function readMapBegin(
    inout ?TType $key_type,
    inout ?TType $val_type,
    inout ?int $size,
  )[zoned_shallow]: int {
    $key_type_byte = -1;
    $key_res = $this->readByte(inout $key_type_byte);
    $key_type = TType::assert($key_type_byte);
    $val_type_byte = -1;
    $val_res = $this->readByte(inout $val_type_byte);
    $val_type = TType::assert($val_type_byte);
    // TODO (partisan): Remove after refactoring of read methods to not use
    // inout.
    $size_nonnull = -1;
    $size_res = $this->readI32(inout $size_nonnull);
    $size = $size_nonnull;
    return $key_res + $val_res + $size_res;
  }

  <<__Override>>
  public function readMapEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function readListBegin(
    inout ?TType $elem_type,
    inout ?int $size,
  )[zoned_shallow]: int {
    $elem_type = -1;
    $etype_res = $this->readByte(inout $elem_type);
    $elem_type = TType::assert($elem_type);
    // TODO (partisan): Remove after refactoring of read methods to not use
    // inout.
    $size_nonnull = -1;
    $size_res = $this->readI32(inout $size_nonnull);
    $size = $size_nonnull;
    return $etype_res + $size_res;
  }

  <<__Override>>
  public function readListEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function readSetBegin(
    inout ?TType $elem_type,
    inout ?int $size,
  )[zoned_shallow]: int {
    $elem_type_byte = -1;
    $etype_res = $this->readByte(inout $elem_type_byte);
    $elem_type = TType::assert($elem_type_byte);
    // TODO (partisan): Remove after refactoring of read methods to not use
    // inout.
    $size_nonnull = -1;
    $size_res = $this->readI32(inout $size_nonnull);
    $size = $size_nonnull;
    return $etype_res + $size_res;
  }

  <<__Override>>
  public function readSetEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function readBool(inout bool $value)[zoned_shallow]: int {
    $data = $this->trans_->readAll(1);
    $arr = PHP\unpack('c', $data);
    $value = $arr[1] == 1;
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

  <<__Override>>
  public function readI16(inout int $value)[zoned_shallow]: int {
    $data = $this->trans_->readAll(2);
    $value = PHP\ord($data[1]) | (PHP\ord($data[0]) << 8);
    if ($value > 0x7fff) {
      $value = 0 - (($value - 1) ^ 0xffff);
    }
    return 2;
  }

  <<__Override>>
  public function readI32(inout int $value)[zoned_shallow]: int {
    $data = $this->trans_->readAll(4);
    $value = $this->unpackI32($data);
    return 4;
  }

  <<__Override>>
  public function readI64(inout int $value)[zoned_shallow]: int {
    $data = $this->trans_->readAll(8);

    $arr = PHP\unpack('N2', $data);

    // If we are on a 32bit architecture we have to explicitly deal with
    // 64-bit twos-complement arithmetic since PHP wants to treat all ints
    // as signed and any int over 2^31 - 1 as a float
    if (PHP_INT_SIZE === 4) {
      $hi = $arr[1] as int;
      $lo = $arr[2] as int;
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

      // Force 32bit words in excess of 2G to pe positive - we deal wigh sign
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
        $value as int;
      } else {
        $value = $arr[1] * 4294967296 + $arr[2];
        $value as int;
      }
    }

    return 8;
  }

  <<__Override>>
  public function readDouble(inout float $value)[zoned_shallow]: int {
    $data = $this->trans_->readAll(8);
    if ($this->littleEndian) {
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
    if ($this->littleEndian) {
      $data = PHP\strrev($data);
    }
    $arr = PHP\unpack('f', $data);
    $value = $arr[1];
    $value as float;
    return 4;
  }

  <<__Override>>
  public function readString(inout string $value)[zoned_shallow]: int {
    $len = 0;
    $result = $this->readI32(inout $len);
    if ($len) {
      $value = $this->trans_->readAll($len);
    } else {
      $value = '';
    }
    return $result + $len;
  }
}

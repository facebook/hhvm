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
 *
 * @package thrift.protocol.binary
 */

/**
 * Binary implementation of the Thrift protocol.
 */
abstract class TBinaryProtocolBase extends TProtocol {

  const VERSION_MASK = 0xffff0000;
  const VERSION_1 = 0x80010000;

  protected $strictRead_ = false;
  protected $strictWrite_ = true;
  protected $littleendian_ = false;
  protected $memory_limit = 128000000; //128M, the default
  protected $sequenceID = null;

  public function __construct(
    $trans,
    $strictRead = false,
    $strictWrite = true,
  ) {
    parent::__construct($trans);
    $this->strictRead_ = $strictRead;
    $this->strictWrite_ = $strictWrite;
    if (pack('S', 1) == "\x01\x00") {
      $this->littleendian_ = true;
    }
    $this->memory_limit = self::getBytes(ini_get('memory_limit'));
  }

  // helper function to get integer from potential php short notation
  public static function getBytes($notation) {
    $val = trim($notation);
    $last = strtolower($val[strlen($val) - 1]);
    switch ($last) {
      // The 'G' modifier is available since PHP 5.1.0
      case 'g':
        $val *= 1024;
        // FALLTHROUGH
      case 'm':
        $val *= 1024;
        // FALLTHROUGH
      case 'k':
        $val *= 1024;
    }

    return $val;
  }

  public function writeMessageBegin($name, $type, $seqid) {
    if ($this->strictWrite_) {
      $version = self::VERSION_1 | $type;
      return
        $this->writeI32($version) +
        $this->writeString($name) +
        $this->writeI32($seqid);
    } else {
      return
        $this->writeString($name) +
        $this->writeByte($type) +
        $this->writeI32($seqid);
    }
  }

  public function writeMessageEnd() {
    return 0;
  }

  public function writeStructBegin($name) {
    return 0;
  }

  public function writeStructEnd() {
    return 0;
  }

  public function writeFieldBegin($fieldName, $fieldType, $fieldId) {
    return $this->writeByte($fieldType) + $this->writeI16($fieldId);
  }

  public function writeFieldEnd() {
    return 0;
  }

  public function writeFieldStop() {
    return $this->writeByte(TType::STOP);
  }

  public function writeMapBegin($keyType, $valType, $size) {
    return
      $this->writeByte($keyType) +
      $this->writeByte($valType) +
      $this->writeI32($size);
  }

  public function writeMapEnd() {
    return 0;
  }

  public function writeListBegin($elemType, $size) {
    return $this->writeByte($elemType) + $this->writeI32($size);
  }

  public function writeListEnd() {
    return 0;
  }

  public function writeSetBegin($elemType, $size) {
    return $this->writeByte($elemType) + $this->writeI32($size);
  }

  public function writeSetEnd() {
    return 0;
  }

  public function writeBool($value) {
    $data = pack('c', $value ? 1 : 0);
    $this->trans_->write($data);
    return 1;
  }

  public function writeByte($value) {
    $this->trans_->write(chr($value));
    return 1;
  }

  public function writeI16($value) {
    $data = chr($value).chr($value >> 8);
    if ($this->littleendian_) {
      $data = strrev($data);
    }
    $this->trans_->write($data);
    return 2;
  }

  public function writeI32($value) {
    $data = chr($value).chr($value >> 8).chr($value >> 16).chr($value >> 24);
    if ($this->littleendian_) {
      $data = strrev($data);
    }
    $this->trans_->write($data);
    return 4;
  }

  public function writeI64($value) {
    $data = '';
    // If we are on a 32bit architecture we have to explicitly deal with
    // 64-bit twos-complement arithmetic since PHP wants to treat all ints
    // as signed and any int over 2^31 - 1 as a float
    if (PHP_INT_SIZE == 4) {
      $neg = $value < 0;

      if ($neg) {
        $value *= -1;
      }

      $hi = (int) ($value / 4294967296);
      $lo = (int) $value;

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
      $data = pack('N2', $hi, $lo);

    } else {
      $data =
        chr($value).
        chr($value >> 8).
        chr($value >> 16).
        chr($value >> 24).
        chr($value >> 32).
        chr($value >> 40).
        chr($value >> 48).
        chr($value >> 56);
      if ($this->littleendian_) {
        $data = strrev($data);
      }
    }

    $this->trans_->write($data);
    return 8;
  }

  public function writeDouble($value) {
    $data = pack('d', $value);
    if ($this->littleendian_) {
      $data = strrev($data);
    }
    $this->trans_->write($data);
    return 8;
  }

  public function writeFloat($value) {
    $data = pack('f', $value);
    if ($this->littleendian_) {
      $data = strrev($data);
    }
    $this->trans_->write($data);
    return 4;
  }

  public function writeString($value) {
    $len = strlen($value);
    $result = $this->writeI32($len);
    if ($len) {
      $this->trans_->write($value);
    }
    return $result + $len;
  }

  private function unpackI32($data) {
    $value =
      ord($data[3]) |
      (ord($data[2]) << 8) |
      (ord($data[1]) << 16) |
      (ord($data[0]) << 24);
    if ($value > 0x7fffffff) {
      $value = 0 - (($value - 1) ^ 0xffffffff);
    }
    return $value;
  }

  /**
   * Returns the sequence ID of the next message; only valid when called
   * before readMessageBegin()
   */
  public function peekSequenceID() {
    $trans = $this->trans_;
    if (!($trans instanceof IThriftBufferedTransport)) {
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
      if ($version != self::VERSION_1) {
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
      if ($this->strictRead_) {
        throw new TProtocolException(
          'No version identifier, old protocol client?',
          TProtocolException::BAD_VERSION,
        );
      } else {
        // need to guard the length from mis-configured other type of TCP server
        // for example, if mis-configure sshd, will read 'SSH-' as the length
        // if memory limit is -1, means no limit.
        if ($this->memory_limit > 0 && $sz > $this->memory_limit) {
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

  public function readMessageBegin(inout $name, inout $type, inout $seqid) {
    $sz = 0;
    $result = $this->readI32($sz);
    if ($sz < 0) {
      $version = $sz & self::VERSION_MASK;
      if ($version != self::VERSION_1) {
        throw new TProtocolException(
          'Bad version identifier: '.$sz,
          TProtocolException::BAD_VERSION,
        );
      }
      $type = $sz & 0x000000ff;
      $result += $this->readString($name) + $this->readI32($seqid);
    } else {
      if ($this->strictRead_) {
        throw new TProtocolException(
          'No version identifier, old protocol client?',
          TProtocolException::BAD_VERSION,
        );
      } else {
        // need to guard the length from mis-configured other type of TCP server
        // for example, if mis-configure sshd, will read 'SSH-' as the length
        // if memory limit is -1, means no limit.
        if ($this->memory_limit > 0 && $sz > $this->memory_limit) {
          throw new TProtocolException(
            'Length overflow: '.$sz,
            TProtocolException::SIZE_LIMIT,
          );
        }
        // Handle pre-versioned input
        $name = $this->trans_->readAll($sz);
        $result += $sz + $this->readByte($type) + $this->readI32($seqid);
      }
    }
    $this->sequenceID = $seqid;
    return $result;
  }

  public function readMessageEnd() {
    $this->sequenceID = null;
    return 0;
  }

  public function readStructBegin(inout $name) {
    $name = '';
    return 0;
  }

  public function readStructEnd() {
    return 0;
  }

  public function readFieldBegin(
    inout $name,
    inout $fieldType,
    inout $fieldId,
  ) {
    $result = $this->readByte($fieldType);
    if ($fieldType == TType::STOP) {
      $fieldId = 0;
      return $result;
    }
    $result += $this->readI16($fieldId);
    return $result;
  }

  public function readFieldEnd() {
    return 0;
  }

  public function readMapBegin(inout $keyType, inout $valType, inout $size) {
    return
      $this->readByte($keyType) +
      $this->readByte($valType) +
      $this->readI32($size);
  }

  public function readMapEnd() {
    return 0;
  }

  public function readListBegin(&$elemType, &$size) {
    return $this->readByte($elemType) + $this->readI32($size);
  }

  public function readListEnd() {
    return 0;
  }

  public function readSetBegin(&$elemType, &$size) {
    return $this->readByte($elemType) + $this->readI32($size);
  }

  public function readSetEnd() {
    return 0;
  }

  public function readBool(&$value) {
    $data = $this->trans_->readAll(1);
    $arr = unpack('c', $data);
    $value = $arr[1] == 1;
    return 1;
  }

  public function readByte(&$value) {
    $data = $this->trans_->readAll(1);
    $value = ord($data);
    if ($value > 0x7f) {
      $value = 0 - (($value - 1) ^ 0xff);
    }
    return 1;
  }

  public function readI16(&$value) {
    $data = $this->trans_->readAll(2);
    $value = ord($data[1]) | (ord($data[0]) << 8);
    if ($value > 0x7fff) {
      $value = 0 - (($value - 1) ^ 0xffff);
    }
    return 2;
  }

  public function readI32(&$value) {
    $data = $this->trans_->readAll(4);
    $value = $this->unpackI32($data);
    return 4;
  }

  public function readI64(&$value) {
    $data = $this->trans_->readAll(8);

    $arr = unpack('N2', $data);

    // If we are on a 32bit architecture we have to explicitly deal with
    // 64-bit twos-complement arithmetic since PHP wants to treat all ints
    // as signed and any int over 2^31 - 1 as a float
    if (PHP_INT_SIZE == 4) {

      $hi = $arr[1];
      $lo = $arr[2];
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

      // Force 32bit words in excess of 2G to pe positive - we deal wigh sign
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

    return 8;
  }

  public function readDouble(&$value) {
    $data = $this->trans_->readAll(8);
    if ($this->littleendian_) {
      $data = strrev($data);
    }
    $arr = unpack('d', $data);
    $value = $arr[1];
    return 8;
  }

  public function readFloat(&$value) {
    $data = $this->trans_->readAll(4);
    if ($this->littleendian_) {
      $data = strrev($data);
    }
    $arr = unpack('f', $data);
    $value = $arr[1];
    return 4;
  }

  public function readString(&$value) {
    $len = 0;
    $result = $this->readI32($len);
    if ($len) {
      $value = $this->trans_->readAll($len);
    } else {
      $value = '';
    }
    return $result + $len;
  }
}

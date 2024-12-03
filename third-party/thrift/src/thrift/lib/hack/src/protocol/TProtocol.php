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
 * Protocol base class module.
 */
<<Oncalls('thrift')>> // @oss-disable
abstract class TProtocol {
  // The widest context used by a protocol when considering all transports used.
  abstract const ctx CReadWriteDefault super [zoned_shallow];

  private int $options = 0;

  protected function __construct(protected TTransport $trans_)[] {
  }

  public function getTransport()[]: TTransport {
    return $this->trans_;
  }

  final public function setOptions(int $options)[write_props]: this {
    $this->options = $options;
    return $this;
  }

  final public function getOptions()[]: int {
    return $this->options;
  }

  /**
   * Writes the message header
   *
   * @param string $name Function name
   * @param int $type message type TMessageType::CALL or TMessageType::REPLY
   * @param int $seqid The sequence id of this message
   */
  public abstract function writeMessageBegin(
    string $name,
    TMessageType $type,
    int $seqid,
  )[this::CReadWriteDefault]: int;

  /**
   * Close the message
   */
  public abstract function writeMessageEnd()[write_props]: int;

  /**
   * Writes a struct header.
   *
   * @param string     $name Struct name
   * @throws TException on write error
   * @return int How many bytes written
   */
  public abstract function writeStructBegin(string $name)[write_props]: int;

  /**
   * Close a struct.
   *
   * @throws TException on write error
   * @return int How many bytes written
   */
  public abstract function writeStructEnd()[write_props]: int;

  /*
   * Starts a field.
   *
   * @param string     $name Field name
   * @param TType        $type Field type
   * @param int        $fid  Field id
   * @throws TException on write error
   * @return int How many bytes written
   */
  public abstract function writeFieldBegin(
    string $field_name,
    TType $field_type,
    int $field_id,
  )[this::CReadWriteDefault]: int;

  public abstract function writeFieldEnd()[write_props]: int;

  public abstract function writeFieldStop()[this::CReadWriteDefault]: int;

  public abstract function writeMapBegin(
    TType $key_type,
    TType $val_type,
    int $size,
  )[this::CReadWriteDefault]: int;

  public abstract function writeMapEnd()[write_props]: int;

  public abstract function writeListBegin(
    TType $elem_type,
    int $size,
  )[this::CReadWriteDefault]: int;

  public abstract function writeListEnd()[write_props]: int;

  public abstract function writeSetBegin(
    TType $elem_type,
    int $size,
  )[this::CReadWriteDefault]: int;

  public abstract function writeSetEnd()[write_props]: int;

  public abstract function writeBool(bool $bool)[this::CReadWriteDefault]: int;

  public abstract function writeByte(int $byte)[this::CReadWriteDefault]: int;

  public abstract function writeI16(int $i16)[this::CReadWriteDefault]: int;

  public abstract function writeI32(int $i32)[this::CReadWriteDefault]: int;

  public abstract function writeI64(int $i64)[this::CReadWriteDefault]: int;

  public abstract function writeDouble(
    float $dub,
  )[this::CReadWriteDefault]: int;

  public abstract function writeFloat(float $flt)[this::CReadWriteDefault]: int;

  public abstract function writeString(
    string $str,
  )[this::CReadWriteDefault]: int;

  public function writeBinary(string $str)[this::CReadWriteDefault]: int {
    return $this->writeString($str);
  }

  /**
   * Reads the message header
   *
   * @param string $name Function name
   * @param int $type message type TMessageType::CALL or TMessageType::REPLY
   * @param int $seqid The sequence id of this message
   */
  public abstract function readMessageBegin(
    inout string $name,
    inout int $type,
    inout int $seqid,
  )[this::CReadWriteDefault]: int;

  /**
   * Read the close of message
   */
  public abstract function readMessageEnd()[write_props]: int;

  public abstract function readStructBegin(
    inout ?string $name,
  )[write_props]: int;

  public abstract function readStructEnd()[write_props]: int;

  public abstract function readFieldBegin(
    inout ?string $name,
    inout ?TType $field_type,
    inout ?int $field_id,
  )[this::CReadWriteDefault]: int;

  public abstract function readFieldEnd()[write_props]: int;

  public abstract function readMapBegin(
    inout ?TType $key_type,
    inout ?TType $val_type,
    inout ?int $size,
  )[this::CReadWriteDefault]: int;

  public function readMapHasNext()[write_props]: bool {
    throw new TProtocolException(
      nameof static.' does not support unknown map sizes',
    );
  }

  public abstract function readMapEnd()[write_props]: int;

  public abstract function readListBegin(
    inout ?TType $elem_type,
    inout ?int $size,
  )[this::CReadWriteDefault]: int;

  public function readListHasNext()[write_props]: bool {
    throw new TProtocolException(
      nameof static.' does not support unknown list sizes',
    );
  }

  public abstract function readListEnd()[write_props]: int;

  public abstract function readSetBegin(
    inout ?TType $elem_type,
    inout ?int $size,
  )[this::CReadWriteDefault]: int;

  public function readSetHasNext()[write_props]: bool {
    throw new TProtocolException(
      nameof static.' does not support unknown set sizes',
    );
  }

  public abstract function readSetEnd()[write_props]: int;

  public abstract function readBool(
    inout bool $bool,
  )[this::CReadWriteDefault]: int;

  public abstract function readByte(
    inout int $byte,
  )[this::CReadWriteDefault]: int;

  public abstract function readI16(
    inout int $i16,
  )[this::CReadWriteDefault]: int;

  public abstract function readI32(
    inout int $i32,
  )[this::CReadWriteDefault]: int;

  public abstract function readI64(
    inout int $i64,
  )[this::CReadWriteDefault]: int;

  public abstract function readDouble(
    inout float $dub,
  )[this::CReadWriteDefault]: int;

  public abstract function readFloat(
    inout float $flt,
  )[this::CReadWriteDefault]: int;

  public abstract function readString(
    inout string $str,
  )[this::CReadWriteDefault]: int;

  public function readBinary(inout string $str)[this::CReadWriteDefault]: int {
    return $this->readString(inout $str);
  }

  /**
   * The skip function is a utility to parse over unrecognized date without
   * causing corruption.
   *
   * @param TType $type What type is it
   */
  public function skip(TType $type)[this::CReadWriteDefault, write_props]: int {
    switch ($type) {
      case TType::BOOL:
        $_ref = false;
        $result = $this->readBool(inout $_ref);
        return $result;
      case TType::BYTE:
        $_ref = 0;
        $result = $this->readByte(inout $_ref);
        return $result;
      case TType::I16:
        $_ref = 0;
        $result = $this->readI16(inout $_ref);
        return $result;
      case TType::I32:
        $_ref = 0;
        $result = $this->readI32(inout $_ref);
        return $result;
      case TType::I64:
        $_ref = 0;
        $result = $this->readI64(inout $_ref);
        return $result;
      case TType::DOUBLE:
        $_ref = 0.0;
        $result = $this->readDouble(inout $_ref);
        return $result;
      case TType::FLOAT:
        $_ref = 0.0;
        $result = $this->readFloat(inout $_ref);
        return $result;
      case TType::STRING:
        $_ref = '';
        $result = $this->readString(inout $_ref);
        return $result;
      case TType::STRUCT:
        $_ref_struct = null;
        $result = $this->readStructBegin(inout $_ref_struct);
        while (true) {
          $_ref_field = null;
          $ftype = null;
          $_ref_field_id = null;
          $field_res = $this->readFieldBegin(
            inout $_ref_field,
            inout $ftype,
            inout $_ref_field_id,
          );
          $result += $field_res;
          if ($ftype === TType::STOP) {
            break;
          }
          $result += $this->skip(nullthrows($ftype, 'Got unexpected null'));
          $result += $this->readFieldEnd();
        }
        $result += $this->readStructEnd();
        return $result;
      case TType::MAP: {
        $key_type = null;
        $val_type = null;
        $size = 0;
        $result =
          $this->readMapBegin(inout $key_type, inout $val_type, inout $size);
        for ($i = 0; $size === null || $i < $size; $i++) {
          if ($size === null && !$this->readMapHasNext()) {
            break;
          }
          $result += $this->skip(nullthrows($key_type, 'Got unexpected null'));
          $result += $this->skip(nullthrows($val_type, 'Got unexpected null'));
        }
        $result += $this->readMapEnd();
        return $result;
      }
      case TType::SET: {
        $elem_type = null;
        $size = 0;
        $result = $this->readSetBegin(inout $elem_type, inout $size);
        for ($i = 0; $size === null || $i < $size; $i++) {
          if ($size === null && !$this->readSetHasNext()) {
            break;
          }
          $result += $this->skip(nullthrows($elem_type, 'Got unexpected null'));
        }
        $result += $this->readSetEnd();
        return $result;
      }
      case TType::LST: {
        $elem_type = null;
        $size = 0;
        $result = $this->readListBegin(inout $elem_type, inout $size);
        for ($i = 0; $size === null || $i < $size; $i++) {
          if ($size === null && !$this->readSetHasNext()) {
            break;
          }
          $result += $this->skip(nullthrows($elem_type, 'Got unexpected null'));
        }
        $result += $this->readListEnd();
        return $result;
      }
      default:
        throw new TProtocolException(
          'Unknown field type: '.$type,
          TProtocolException::INVALID_DATA,
        );
    }
  }

  /**
   * Utility for skipping binary data
   *
   * @param TTransport $itrans TTransport object
   * @param int        $type   Field type
   */
  public static function skipBinary(
    TTransport $itrans,
    TType $type,
  )[zoned_shallow]: int {
    switch ($type) {
      case TType::BOOL:
        return Str\length($itrans->readAll(1));
      case TType::BYTE:
        return Str\length($itrans->readAll(1));
      case TType::I16:
        return Str\length($itrans->readAll(2));
      case TType::I32:
        return Str\length($itrans->readAll(4));
      case TType::I64:
        return Str\length($itrans->readAll(8));
      case TType::DOUBLE:
        return Str\length($itrans->readAll(8));
      case TType::FLOAT:
        return Str\length($itrans->readAll(4));
      case TType::STRING:
        $len = PHP\unpack('N', $itrans->readAll(4));
        $len = $len[1];
        if ($len > 0x7fffffff) {
          $len = 0 - (($len - 1) ^ 0xffffffff);
        }
        return 4 + Str\length($itrans->readAll($len));
      case TType::STRUCT: {
        $result = 0;
        while (true) {
          $data = $itrans->readAll(1);
          $arr = PHP\unpack('c', $data);
          $ftype = $arr[1];
          if ($ftype === TType::STOP) {
            break;
          }
          // I16 field id
          $result += Str\length($itrans->readAll(2));
          $result += self::skipBinary($itrans, HH\FIXME\UNSAFE_CAST<
            dynamic,
            TType,
          >($ftype, 'Exposed as unpack may return false with invalid input'));
        }
        return $result;
      }
      case TType::MAP: {
        // Ktype
        $data = $itrans->readAll(1);
        $arr = PHP\unpack('c', $data);
        $ktype = $arr[1];
        // Vtype
        $data = $itrans->readAll(1);
        $arr = PHP\unpack('c', $data);
        $vtype = $arr[1];
        // Size
        $data = $itrans->readAll(4);
        $arr = PHP\unpack('N', $data);
        $size = $arr[1];
        if ($size > 0x7fffffff) {
          $size = 0 - (($size - 1) ^ 0xffffffff);
        }
        $result = 6;
        for ($i = 0; $i < $size; $i++) {
          $result += self::skipBinary($itrans, HH\FIXME\UNSAFE_CAST<
            dynamic,
            TType,
          >($ktype, 'Exposed as unpack may return false with invalid input'));
          $result += self::skipBinary($itrans, HH\FIXME\UNSAFE_CAST<
            dynamic,
            TType,
          >($vtype, 'Exposed as unpack may return false with invalid input'));
        }
        return $result;
      }
      case TType::SET:
      case TType::LST: {
        // Vtype
        $data = $itrans->readAll(1);
        $arr = PHP\unpack('c', $data);
        $vtype = $arr[1];
        // Size
        $data = $itrans->readAll(4);
        $arr = PHP\unpack('N', $data);
        $size = $arr[1];
        if ($size > 0x7fffffff) {
          $size = 0 - (($size - 1) ^ 0xffffffff);
        }
        $result = 5;
        for ($i = 0; $i < $size; $i++) {
          $result += self::skipBinary($itrans, HH\FIXME\UNSAFE_CAST<
            dynamic,
            TType,
          >($vtype, 'Exposed as unpack may return false with invalid input'));
        }
        return $result;
      }
      default:
        throw new TProtocolException(
          'Unknown field type: '.$type,
          TProtocolException::INVALID_DATA,
        );
    }
  }
}

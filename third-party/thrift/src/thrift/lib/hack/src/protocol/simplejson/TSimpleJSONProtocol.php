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
 * Protocol for encoding/decoding simple json
 */
<<Oncalls('thrift')>> // @oss-disable
class TSimpleJSONProtocol extends TProtocol {
  const ctx CReadWriteDefault = [write_props];

  const VERSION_1 = 0x80010000;

  // String constants for floating point types that aren't valid JSON.
  const THRIFT_NAN = "NaN";
  const THRIFT_NEGATIVE_NAN = "-NaN";
  const THRIFT_INFINITY = "Infinity";
  const THRIFT_NEGATIVE_INFINITY = "-Infinity";

  protected TMemoryBuffer $buffer;
  private Vector<TSimpleJSONProtocolContext> $contexts;
  private bool $binaryAsBase64;

  // If the serialized data has just the number,
  // then deserialization breaks as it reaches the end of the string.
  // This flag will ensure that deserializer stops reading
  // as soon as the end of buffer is reached.
  private bool $specialHandlingForNumber = false;

  public function __construct(TMemoryBuffer $trans)[] {
    parent::__construct($trans);
    $this->buffer = $trans;
    $this->contexts = Vector {
      new TSimpleJSONProtocolContext($this->buffer),
    };
    $this->binaryAsBase64 = false;
  }

  public function setSpecialHandlingForNumber(bool $value)[write_props]: this {
    $this->specialHandlingForNumber = $value;
    return $this;
  }

  public function setBinaryAsBase64(bool $value)[write_props]: this {
    $this->binaryAsBase64 = $value;
    return $this;
  }

  protected function pushListWriteContext()[write_props]: int {
    return $this->pushWriteContext(
      new TSimpleJSONProtocolListContext($this->buffer),
    );
  }

  protected function pushMapWriteContext()[write_props]: int {
    return
      $this->pushWriteContext(new TSimpleJSONProtocolMapContext($this->buffer));
  }

  protected function pushListReadContext()[write_props]: int {
    return
      $this->pushReadContext(new TSimpleJSONProtocolListContext($this->buffer));
  }

  protected function pushMapReadContext()[write_props]: int {
    return
      $this->pushReadContext(new TSimpleJSONProtocolMapContext($this->buffer));
  }

  protected function pushWriteContext(
    TSimpleJSONProtocolContext $ctx,
  )[write_props]: int {
    $this->contexts->add($ctx);
    return $ctx->writeStart();
  }

  protected function popWriteContext()[write_props]: int {
    $ctx = $this->contexts->pop();
    return $ctx->writeEnd();
  }

  protected function pushReadContext(
    TSimpleJSONProtocolContext $ctx,
  )[write_props]: int {
    $this->contexts->add($ctx);
    return $ctx->readStart();
  }

  protected function popReadContext()[write_props]: int {
    $ctx = $this->contexts->pop();
    return $ctx->readEnd();
  }

  protected function getContext()[]: TSimpleJSONProtocolContext {
    return $this->contexts->at($this->contexts->count() - 1);
  }

  <<__Override>>
  public function writeMessageBegin(
    string $name,
    TMessageType $type,
    int $seqid,
  )[write_props]: int {
    return $this->getContext()->writeSeparator() +
      $this->pushListWriteContext() +
      $this->writeI32(self::VERSION_1) +
      $this->writeString($name) +
      $this->writeI32($type) +
      $this->writeI32($seqid);
  }

  <<__Override>>
  public function writeMessageEnd()[write_props]: int {
    return $this->popWriteContext();
  }

  <<__Override>>
  public function writeStructBegin(string $_name)[write_props]: int {
    return $this->getContext()->writeSeparator() + $this->pushMapWriteContext();
  }

  <<__Override>>
  public function writeStructEnd()[write_props]: int {
    return $this->popWriteContext();
  }

  <<__Override>>
  public function writeFieldBegin(
    string $field_name,
    TType $_field_type,
    int $_field_id,
  )[write_props]: int {
    return $this->writeString($field_name);
  }

  <<__Override>>
  public function writeFieldEnd()[]: int {
    return 0;
  }

  <<__Override>>
  public function writeFieldStop()[]: int {
    return 0;
  }

  <<__Override>>
  public function writeMapBegin(
    TType $_key_type,
    TType $_val_type,
    int $_size,
  )[write_props]: int {
    return $this->getContext()->writeSeparator() + $this->pushMapWriteContext();
  }

  <<__Override>>
  public function writeMapEnd()[write_props]: int {
    return $this->popWriteContext();
  }

  <<__Override>>
  public function writeListBegin(
    TType $_elem_type,
    int $_size,
  )[write_props]: int {
    return
      $this->getContext()->writeSeparator() + $this->pushListWriteContext();
  }

  <<__Override>>
  public function writeListEnd()[write_props]: int {
    return $this->popWriteContext();
  }

  <<__Override>>
  public function writeSetBegin(
    TType $_elem_type,
    int $_size,
  )[write_props]: int {
    return
      $this->getContext()->writeSeparator() + $this->pushListWriteContext();
  }

  <<__Override>>
  public function writeSetEnd()[write_props]: int {
    return $this->popWriteContext();
  }

  <<__Override>>
  public function writeBool(bool $value)[write_props]: int {
    $x = $this->getContext()->writeSeparator();
    if ($value) {
      $this->buffer->write('true');
      $x += 4;
    } else {
      $this->buffer->write('false');
      $x += 5;
    }
    return $x;
  }

  <<__Override>>
  public function writeByte(int $value)[write_props]: int {
    return $this->writeNum((int)$value);
  }

  <<__Override>>
  public function writeI16(int $value)[write_props]: int {
    return $this->writeNum((int)$value);
  }

  <<__Override>>
  public function writeI32(int $value)[write_props]: int {
    return $this->writeNum((int)$value);
  }

  <<__Override>>
  public function writeI64(int $value)[write_props]: int {
    return $this->writeNum((int)$value);
  }

  <<__Override>>
  public function writeDouble(num $value)[write_props]: int {
    return $this->writeFloatingPoint((float)$value);
  }

  <<__Override>>
  public function writeFloat(float $value)[write_props]: int {
    return $this->writeFloatingPoint((float)$value);
  }

  // Convert the given double to a JSON string, which is either the number,
  // "NaN" or "Infinity" or "-Infinity".
  protected function writeFloatingPoint(float $value)[write_props]: int {
    /* @lint-ignore FLOATING_POINT_COMPARISON */
    if ($value === INF) {
      $str = self::THRIFT_INFINITY;
    /* @lint-ignore FLOATING_POINT_COMPARISON */
    } else if ($value === -INF) {
      $str = self::THRIFT_NEGATIVE_INFINITY;
    } else if (Math\is_nan($value)) {
      $str = self::THRIFT_NAN;
    } else {
      return $this->writeNum($value);
    }

    return $this->writeString($str);
  }

  protected function writeNum(num $value)[write_props]: int {
    $ctx = $this->getContext();
    $ret = $ctx->writeSeparator();
    $value = (string)$value;
    if ($ctx->escapeNum()) {
      $value = '"'.$value.'"';
    }
    $this->buffer->write($value);
    return $ret + Str\length($value);
  }

  <<TestsBypassVisibility>>
  private static function quoteString(string $value)[write_props]: string {
    $sb = '"';
    $len = Str\length($value);
    for ($i = 0; $i < $len; $i++) {
      $c = $value[$i];
      $ord = PHP\ord($c);
      switch ($ord) {
        case 8:
          $sb .= '\b';
          break;
        case 9:
          $sb .= '\t';
          break;
        case 10:
          $sb .= '\n';
          break;
        case 12:
          $sb .= '\f';
          break;
        case 13:
          $sb .= '\r';
          break;
        case 34:
        // "
        case 92:
          // \
          $sb .= '\\';
          $sb .= $c;
          break;
        default:
          if ($ord < 32 || $ord > 126) {
            $sb .= '\\u00';
            $sb .= PHP\bin2hex($c);
          } else {
            $sb .= $c;
          }
          break;
      }
    }
    $sb .= '"';
    return $sb;
  }

  <<__Override>>
  public function writeString(string $value)[write_props]: int {
    $ctx = $this->getContext();
    $ret = $ctx->writeSeparator();
    $value = (string)$value;
    $sb = self::quoteString($value);

    $this->buffer->write($sb);

    return $ret + Str\length($sb);
  }

  <<__Override>>
  public function writeBinary(string $str)[write_props]: int {
    if ($this->binaryAsBase64) {
      $str = Base64::encode($str);
    }
    return $this->writeString($str);
  }

  <<__Override>>
  public function readMessageBegin(
    inout string $_name,
    inout int $_type,
    inout int $_seqid,
  )[]: int {
    throw new TProtocolException(
      'Reading with TSimpleJSONProtocol is not supported. '.
      'Use readFromJSON() on your struct',
    );
  }

  <<__Override>>
  public function readMessageEnd()[]: int {
    throw new TProtocolException(
      'Reading with TSimpleJSONProtocol is not supported. '.
      'Use readFromJSON() on your struct',
    );
  }

  <<__Override>>
  public function readStructBegin(inout ?string $_name)[write_props]: int {
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    $result += $this->pushMapReadContext();
    return $result;
  }

  <<__Override>>
  public function readStructEnd()[write_props]: int {
    $this->popReadContext();
    return 0;
  }

  <<__Override>>
  public function readFieldBegin(
    inout ?string $name,
    inout ?TType $field_type,
    inout ?int $field_id,
  )[write_props]: int {
    $field_id = null;
    $ctx = $this->getContext();
    $name = null;
    $result = 0;
    while ($name === null) {
      if ($ctx->readContextOver()) {
        $field_type = TType::STOP;
        break;
      } else {
        $result += $ctx->readSeparator();
        $result += $this->skipWhitespace();
        $name = $this->readJSONString()[0];
        // We need to guess the type of the value, in case the name is bogus or we are in a skip method up the stack
        $offset = $this->skipWhitespace(true);
        $this->expectChar(':', true, $offset);
        $offset += 1 + $this->skipWhitespace(true, $offset + 1);
        $c = $this->buffer->peek(1, $offset);
        if ($c === 'n' && $this->buffer->peek(4, $offset) === 'null') {
          // We actually want to skip this field, but there isn't an appropriate
          // TType to send back. So instead, we will silently skip
          $result += $ctx->readSeparator();
          $result += $this->skipWhitespace();
          $result += Str\length($this->buffer->readAll(4));
          $name = null;
          continue;
        } else {
          $field_type = $this->guessFieldTypeBasedOnByte($c);
        }
      }
    }
    return $result;
  }

  <<__Override>>
  public function readFieldEnd()[]: int {
    // Do nothing
    return 0;
  }

  <<__Override>>
  public function readMapBegin(
    inout ?TType $key_type,
    inout ?TType $val_type,
    inout ?int $size,
  )[write_props]: int {
    $size = null;
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    $result += $this->pushMapReadContext();
    if ($this->readMapHasNext()) {
      // We need to guess the type of the keys/values, in case we are in a skip
      // method up the stack
      $key_type = TType::STRING;
      // This is not a peek, since we can do this safely again
      $result += $this->skipWhitespace();
      $offset = $this->readJSONString(true)[1];
      $offset += $this->skipWhitespace(true, $offset);
      $this->expectChar(':', true, $offset);
      $offset += 1 + $this->skipWhitespace(true, $offset + 1);
      $c = $this->buffer->peek(1, $offset);
      $val_type = $this->guessFieldTypeBasedOnByte($c);
    }
    return $result;
  }

  <<__Override>>
  public function readMapHasNext()[write_props]: bool {
    return !$this->getContext()->readContextOver();
  }

  <<__Override>>
  public function readMapEnd()[write_props]: int {
    return $this->popReadContext();
  }

  <<__Override>>
  public function readListBegin(
    inout ?TType $elem_type,
    inout ?int $size,
  )[write_props]: int {
    $size = null;
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    $result += $this->pushListReadContext();
    if ($this->readListHasNext()) {
      // We need to guess the type of the values, in case we are in a skip
      // method up the stack.
      // This is not a peek, since we can do this safely again.
      $result += $this->skipWhitespace();
      $c = $this->buffer->peek(1);
      $elem_type = $this->guessFieldTypeBasedOnByte($c);
    }
    return $result;
  }

  <<__Override>>
  public function readListHasNext()[write_props]: bool {
    return !$this->getContext()->readContextOver();
  }

  <<__Override>>
  public function readListEnd()[write_props]: int {
    return $this->popReadContext();
  }

  <<__Override>>
  public function readSetBegin(
    inout ?TType $elem_type,
    inout ?int $size,
  )[write_props]: int {
    $size = null;
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    $result += $this->pushListReadContext();
    if ($this->readSetHasNext()) {
      // We need to guess the type of the values, in case we are in a skip
      // method up the stack.
      // This is not a peek, since we can do this safely again.
      $result += $this->skipWhitespace();
      $c = $this->buffer->peek(1);
      $elem_type = $this->guessFieldTypeBasedOnByte($c);
    }
    return $result;
  }

  <<__Override>>
  public function readSetHasNext()[write_props]: bool {
    return !$this->getContext()->readContextOver();
  }

  <<__Override>>
  public function readSetEnd()[write_props]: int {
    return $this->popReadContext();
  }

  <<__Override>>
  public function readBool(inout bool $value)[write_props]: int {
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    $c = $this->buffer->readAll(1);
    $result++;
    switch ($c) {
      case 't':
        $value = true;
        $target = 'rue';
        break;
      case 'f':
        $value = false;
        $target = 'alse';
        break;
      case '0':
        $value = false;
        $target = '';
        break;
      case '1':
        $value = true;
        $target = '';
        break;
      default:
        throw new TProtocolException(
          'TSimpleJSONProtocol: Expected t or f, encountered 0x'.
          PHP\bin2hex($c),
        );
    }

    for ($i = 0; $i < Str\length($target); $i++) {
      $this->expectChar($target[$i]);
    }
    return $result;
  }

  protected function readInteger(
    ?int $min,
    ?int $max,
  )[write_props]: (int, int) {
    list($str_val, $chars_read) = $this->readNumStr();
    $val = PHP\intval($str_val);
    if (($min !== null && $max !== null) && ($val < $min || $val > $max)) {
      throw new TProtocolException(
        'TProtocolException: value '.$val.' is outside the expected bounds',
      );
    }
    return tuple($val, $chars_read);
  }

  protected function readFloatingPoint()[write_props]: (float, int) {
    $ctx = $this->getContext();

    // Check for NaN, -NaN, Infinity, and -Infinity
    if ($this->buffer->peek(1) === '"') {
      list($str, $result) = $this->readJSONString();
      switch ($str) {
        case self::THRIFT_NAN:
          $num = Math\NAN;
          break;
        case self::THRIFT_NEGATIVE_NAN:
          $num = -Math\NAN;
          break;
        case self::THRIFT_INFINITY:
          $num = INF;
          break;
        case self::THRIFT_NEGATIVE_INFINITY:
          $num = -INF;
          break;
        default:
          if (!$ctx->escapeNum()) {
            throw new TProtocolException(
              'TSimpleJSONProtocol: Numeric data unexpectedly quoted',
            );
          }
          $num = (float)$str;
          break;
      }

      return tuple($num, $result);
    }

    list($str, $result) = $this->readNumStr();
    return tuple((float)$str, $result);
  }

  protected function readNumStr()[write_props]: (string, int) {
    $ctx = $this->getContext();
    if ($ctx->escapeNum()) {
      $this->expectChar('"');
    }
    $count = 0;
    $reading = true;
    while ($reading) {
      if (
        $this->specialHandlingForNumber && $count === $this->buffer->available()
      ) {
        break;
      }
      $c = $this->buffer->peek(1, $count);
      switch ($c) {
        case '+':
        case '-':
        case '.':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'E':
        case 'e':
          $count++;
          break;
        default:
          $reading = false;
          break;
      }
    }
    $str = $this->buffer->readAll($count);
    if (
      !PHP\fb\preg_match_simple(
        '/^[+-]?(?:0|[1-9]\d*)(?:\.\d+)?(?:[eE][+-]?\d+)?$/',
        $str,
      )
    ) {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Invalid json number '.$str,
      );
    }
    if ($ctx->escapeNum()) {
      $this->expectChar('"');
    }

    return tuple($str, $count);
  }

  <<__Override>>
  public function readByte(inout int $value)[write_props]: int {
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    list($value, $chars_read) = $this->readInteger(-0x80, 0x7f);
    $result += $chars_read;
    return $result;
  }

  <<__Override>>
  public function readI16(inout int $value)[write_props]: int {
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    list($value, $chars_read) = $this->readInteger(-0x8000, 0x7fff);
    $result += $chars_read;
    return $result;
  }

  <<__Override>>
  public function readI32(inout int $value)[write_props]: int {
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    list($value, $chars_read) = $this->readInteger(-0x80000000, 0x7fffffff);
    $result += $chars_read;
    return $result;
  }

  <<__Override>>
  public function readI64(inout int $value)[write_props]: int {
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    list($value, $chars_read) = $this->readInteger(null, null);
    $result += $chars_read;
    return $result;
  }

  <<__Override>>
  public function readDouble(inout float $value)[write_props]: int {
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    list($value, $chars_read) = $this->readFloatingPoint();
    $result += $chars_read;
    return $result;
  }

  <<__Override>>
  public function readFloat(inout float $value)[write_props]: int {
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    list($value, $chars_read) = $this->readFloatingPoint();
    $result += $chars_read;
    return $result;
  }

  <<__Override>>
  public function readString(inout string $value)[write_props]: int {
    $result = $this->getContext()->readSeparator();
    $result += $this->skipWhitespace();
    list($value, $chars_read) = $this->readJSONString();
    $result += $chars_read;
    return $result;
  }

  <<__Override>>
  public function readBinary(inout string $str)[write_props]: int {
    $result = $this->readString(inout $str);
    if ($this->binaryAsBase64) {
      $str = Base64::decode($str);
    }
    return $result;
  }

  protected function readJSONString(
    bool $peek = false,
    int $start = 0,
  )[write_props]: (string, int) {
    if (!$peek) {
      $start = 0;
    }
    $this->expectChar('"', $peek, $start);
    $count = $peek ? 1 : 0;
    $sb = '';
    $reading = true;
    while ($reading) {
      $c = $this->buffer->peek(1, $start + $count);
      switch ($c) {
        case '"':
          $reading = false;
          break;
        case '\\':
          $count++;
          $c = $this->buffer->peek(1, $start + $count);
          switch ($c) {
            case '\\':
              $count++;
              $sb .= '\\';
              break;
            case '"':
              $count++;
              $sb .= '"';
              break;
            case 'b':
              $count++;
              $sb .= PHP\chr(0x08);
              break;
            case '/':
              $count++;
              $sb .= '/';
              break;
            case 'f':
              $count++;
              $sb .= "\f";
              break;
            case 'n':
              $count++;
              $sb .= "\n";
              break;
            case 'r':
              $count++;
              $sb .= "\r";
              break;
            case 't':
              $count++;
              $sb .= "\t";
              break;
            case 'u':
              $count++;
              $this->expectChar('0', true, $start + $count);
              $this->expectChar('0', true, $start + $count + 1);
              $count += 2;
              $sb .=
                (string)PHP\hex2bin($this->buffer->peek(2, $start + $count));
              $count += 2;
              break;
            default:
              throw new TProtocolException(
                'TSimpleJSONProtocol: Expected Control Character, found 0x'.
                PHP\bin2hex($c),
              );
          }
          break;
        case '':
          // end of buffer, this string is unclosed
          $reading = false;
          break;
        default:
          $count++;
          $sb .= $c;
          break;
      }
    }

    if (!$peek) {
      $this->buffer->readAll($count);
    }

    $this->expectChar('"', $peek, $start + $count);
    return tuple($sb, $count + 1);
  }

  protected function skipWhitespace(
    bool $peek = false,
    int $start = 0,
  )[write_props]: int {
    if (!$peek) {
      $start = 0;
    }
    $count = 0;
    $reading = true;
    while ($reading) {
      $byte = $this->buffer->peek(1, $count + $start);
      switch ($byte) {
        case ' ':
        case "\t":
        case "\n":
        case "\r":
          $count++;
          break;
        default:
          $reading = false;
          break;
      }
    }
    if (!$peek) {
      $this->buffer->readAll($count);
    }

    return $count;
  }

  protected function expectChar(
    string $char,
    bool $peek = false,
    int $start = 0,
  )[write_props]: void {
    if (!$peek) {
      $start = 0;
    }
    if ($peek) {
      $c = $this->buffer->peek(1, $start);
    } else {
      $c = $this->buffer->readAll(1);
    }

    if ($c !== $char) {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Expected '.
        $char.
        ', but encountered 0x'.
        PHP\bin2hex($c),
      );
    }
  }

  protected function guessFieldTypeBasedOnByte(string $byte)[]: TType {
    switch ($byte) {
      case '{':
        return TType::STRUCT;
      case '[':
        return TType::LST;
      case 't':
      case 'f':
        return TType::BOOL;
      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      // These technically aren't allowed to start JSON floats, but are here
      // for backwards compatibility
      case '+':
      case '.':
        return TType::DOUBLE;
      case '"':
        return TType::STRING;
      case ']':
      case '}':
        // We can get here with empty lists/maps, returning a dummy value
        return TType::STOP;
      /* added due to nonexhaustive switch */
      default:
        break;
    }

    throw new TProtocolException(
      'TSimpleJSONProtocol: Unable to guess TType for character 0x'.
      PHP\bin2hex($byte),
    );
  }
}

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
 * @package thrift.protocol.simplejson
 */

/**
 * Protocol for encoding/decoding simple json
 */
class TSimpleJSONProtocol extends TProtocol {
  const VERSION_1 = 0x80010000;

  private IThriftBufferedTransport $bufTrans;
  private Vector<TSimpleJSONProtocolContext> $contexts;

  public function __construct(TTransport $trans) {
    $this->contexts = Vector {};
    if (!($trans instanceof IThriftBufferedTransport)) {
      $trans = new TBufferedTransport($trans);
    }
    $this->bufTrans = $trans;
    parent::__construct($trans);
    $this->contexts
      ->add(new TSimpleJSONProtocolContext($this->trans_, $this->bufTrans));
  }

  private function pushListWriteContext(): int {
    return $this->pushWriteContext(
      new TSimpleJSONProtocolListContext($this->trans_, $this->bufTrans),
    );
  }

  private function pushMapWriteContext(): int {
    return $this->pushWriteContext(
      new TSimpleJSONProtocolMapContext($this->trans_, $this->bufTrans),
    );
  }

  private function pushListReadContext(): void {
    $this->pushReadContext(
      new TSimpleJSONProtocolListContext($this->trans_, $this->bufTrans),
    );
  }

  private function pushMapReadContext(): void {
    $this->pushReadContext(
      new TSimpleJSONProtocolMapContext($this->trans_, $this->bufTrans),
    );
  }

  private function pushWriteContext(TSimpleJSONProtocolContext $ctx): int {
    $this->contexts->add($ctx);
    return $ctx->writeStart();
  }

  private function popWriteContext(): int {
    $ctx = $this->contexts->pop();
    return $ctx->writeEnd();
  }

  private function pushReadContext(TSimpleJSONProtocolContext $ctx): void {
    $this->contexts->add($ctx);
    $ctx->readStart();
  }

  private function popReadContext(): void {
    $ctx = $this->contexts->pop();
    $ctx->readEnd();
  }

  private function getContext(): TSimpleJSONProtocolContext {
    return $this->contexts->at($this->contexts->count() - 1);
  }

  public function writeMessageBegin($name, $type, $seqid) {
    return
      $this->getContext()->writeSeparator() +
      $this->pushListWriteContext() +
      $this->writeI32(self::VERSION_1) +
      $this->writeString($name) +
      $this->writeI32($type) +
      $this->writeI32($seqid);
  }

  public function writeMessageEnd() {
    return $this->popWriteContext();
  }

  public function writeStructBegin($name) {
    return
      $this->getContext()->writeSeparator() + $this->pushMapWriteContext();
  }

  public function writeStructEnd() {
    return $this->popWriteContext();
  }

  public function writeFieldBegin($fieldName, $fieldType, $fieldId) {
    return $this->writeString($fieldName);
  }

  public function writeFieldEnd() {
    return 0;
  }

  public function writeFieldStop() {
    return 0;
  }

  public function writeMapBegin($keyType, $valType, $size) {
    return
      $this->getContext()->writeSeparator() + $this->pushMapWriteContext();
  }

  public function writeMapEnd() {
    return $this->popWriteContext();
  }

  public function writeListBegin($elemType, $size) {
    return
      $this->getContext()->writeSeparator() + $this->pushListWriteContext();
  }

  public function writeListEnd() {
    return $this->popWriteContext();
  }

  public function writeSetBegin($elemType, $size) {
    return
      $this->getContext()->writeSeparator() + $this->pushListWriteContext();
  }

  public function writeSetEnd() {
    return $this->popWriteContext();
  }

  public function writeBool($value) {
    $x = $this->getContext()->writeSeparator();
    if ($value) {
      $this->trans_->write('true');
      $x += 4;
    } else {
      $this->trans_->write('false');
      $x += 5;
    }
    return $x;
  }

  public function writeByte($value) {
    return $this->writeNum((int) $value);
  }

  public function writeI16($value) {
    return $this->writeNum((int) $value);
  }

  public function writeI32($value) {
    return $this->writeNum((int) $value);
  }

  public function writeI64($value) {
    return $this->writeNum((int) $value);
  }

  public function writeDouble($value) {
    return $this->writeNum((float) $value);
  }

  public function writeFloat($value) {
    return $this->writeNum((float) $value);
  }

  private function writeNum($value) {
    $ctx = $this->getContext();
    $ret = $ctx->writeSeparator();
    if ($ctx->escapeNum()) {
      $value = (string) $value;
    }

    $enc = json_encode($value);
    $this->trans_->write($enc);

    return $ret + strlen($enc);
  }

  public function writeString($value) {
    $ctx = $this->getContext();
    $ret = $ctx->writeSeparator();
    $value = (string) $value;
    $sb = '';
    $sb .= '"';
    $len = strlen($value);
    for ($i = 0; $i < $len; $i++) {
      $c = $value[$i];
      $ord = ord($c);
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
            $sb .= bin2hex($c);
          } else {
            $sb .= $c;
          }
          break;
      }
    }
    $sb .= '"';
    $enc = $sb;
    $this->trans_->write($enc);

    return $ret + strlen($enc);
  }

  public function readMessageBegin(inout $name, inout $type, inout $seqid) {
    throw new TProtocolException(
      'Reading with TSimpleJSONProtocol is not supported. '.
      'Use readFromJSON() on your struct',
    );
  }

  public function readMessageEnd() {
    throw new TProtocolException(
      'Reading with TSimpleJSONProtocol is not supported. '.
      'Use readFromJSON() on your struct',
    );
  }

  public function readStructBegin(inout $_name) {
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $this->pushMapReadContext();
  }

  public function readStructEnd() {
    $this->popReadContext();
  }

  public function readFieldBegin(
    inout $name,
    inout $fieldType,
    inout $fieldId,
  ) {
    $fieldId = null;
    $ctx = $this->getContext();
    $name = null;
    while ($name === null) {
      if ($ctx->readContextOver()) {
        $fieldType = TType::STOP;
        break;
      } else {
        $ctx->readSeparator();
        $this->skipWhitespace();
        $name = $this->readJSONString()[0];
        // We need to guess the type of the value, in case the name is bogus or we are in a skip method up the stack
        $offset = $this->skipWhitespace(true);
        $this->expectChar(':', true, $offset);
        $offset += 1 + $this->skipWhitespace(true, $offset + 1);
        $c = $this->bufTrans->peek(1, $offset);
        if ($c === 'n' && $this->bufTrans->peek(4, $offset) === 'null') {
          // We actually want to skip this field, but there isn't an appropriate
          // TType to send back. So instead, we will silently skip
          $ctx->readSeparator();
          $this->skipWhitespace();
          $this->trans_->readAll(4);
          $name = null;
          continue;
        } else {
          $fieldType = $this->guessFieldTypeBasedOnByte($c);
        }
      }
    }
  }

  public function readFieldEnd() {
    // Do nothing
  }

  public function readMapBegin(inout $keyType, inout $valType, inout $size) {
    $size = null;
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $this->pushMapReadContext();
    if ($this->readMapHasNext()) {
      // We need to guess the type of the keys/values, in case we are in a skip method up the stack
      $keyType = TType::STRING;
      $this->skipWhitespace(); // This is not a peek, since we can do this safely again
      $offset = $this->readJSONString(true)[1];
      $offset += $this->skipWhitespace(true, $offset);
      $this->expectChar(':', true, $offset);
      $offset += 1 + $this->skipWhitespace(true, $offset + 1);
      $c = $this->bufTrans->peek(1, $offset);
      $valType = $this->guessFieldTypeBasedOnByte($c);
    }
  }

  public function readMapHasNext(): bool {
    return !$this->getContext()->readContextOver();
  }

  public function readMapEnd() {
    $this->popReadContext();
  }

  public function readListBegin(&$elemType, &$size) {
    $size = null;
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $this->pushListReadContext();
    if ($this->readListHasNext()) {
      // We need to guess the type of the values, in case we are in a skip method up the stack
      $this->skipWhitespace(); // This is not a peek, since we can do this safely again
      $c = $this->bufTrans->peek(1);
      $elemType = $this->guessFieldTypeBasedOnByte($c);
    }
  }

  public function readListHasNext(): bool {
    return !$this->getContext()->readContextOver();
  }

  public function readListEnd() {
    $this->popReadContext();
  }

  public function readSetBegin(&$elemType, &$size) {
    $size = null;
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $this->pushListReadContext();
    if ($this->readSetHasNext()) {
      // We need to guess the type of the values, in case we are in a skip method up the stack
      $this->skipWhitespace(); // This is not a peek, since we can do this safely again
      $c = $this->bufTrans->peek(1);
      $elemType = $this->guessFieldTypeBasedOnByte($c);
    }
  }

  public function readSetHasNext(): bool {
    return !$this->getContext()->readContextOver();
  }

  public function readSetEnd() {
    $this->popReadContext();
  }

  public function readBool(&$value) {
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $c = $this->trans_->readAll(1);
    $target = null;
    switch ($c) {
      case 't':
        $value = true;
        $target = 'rue';
        break;
      case 'f':
        $value = false;
        $target = 'alse';
        break;
      default:
        throw new TProtocolException(
          'TSimpleJSONProtocol: Expected t or f, encountered 0x'.
          bin2hex($c),
        );
    }

    for ($i = 0; $i < strlen($target); $i++) {
      $this->expectChar($target[$i]);
    }
  }

  private function readInteger(?int $min, ?int $max): int {
    $val = intval($this->readNumStr());
    if (($min !== null && $max !== null) && ($val < $min || $val > $max)) {
      throw new TProtocolException(
        'TProtocolException: value '.
        $val.
        ' is outside the expected bounds',
      );
    }
    return $val;
  }

  private function readNumStr(): string {
    $ctx = $this->getContext();
    if ($ctx->escapeNum()) {
      $this->expectChar('"');
    }
    $count = 0;
    $reading = true;
    while ($reading) {
      $c = $this->bufTrans->peek(1, $count);
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
    $str = $this->trans_->readAll($count);
    if (!preg_match(
          '/^[+-]?(?:0|[1-9]\d*)(?:\.\d+)?(?:[eE][+-]?\d+)?$/',
          $str,
        )) {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Invalid json number '.$str,
      );
    }
    if ($ctx->escapeNum()) {
      $this->expectChar('"');
    }

    return $str;
  }

  public function readByte(&$value) {
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $value = $this->readInteger(-0x80, 0x7f);
  }

  public function readI16(&$value) {
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $value = $this->readInteger(-0x8000, 0x7fff);
  }

  public function readI32(&$value) {
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $value = $this->readInteger(-0x80000000, 0x7fffffff);
  }

  public function readI64(&$value) {
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $value = $this->readInteger(null, null);
  }

  public function readDouble(&$value) {
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $value = (float)$this->readNumStr();
  }

  public function readFloat(&$value) {
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $value = (float)$this->readNumStr();
  }

  public function readString(&$value) {
    $this->getContext()->readSeparator();
    $this->skipWhitespace();
    $value = $this->readJSONString()[0];
  }

  private function readJSONString(
    bool $peek = false,
    int $start = 0,
  ): Pair<string, int> {
    if (!$peek) {
      $start = 0;
    }
    $this->expectChar('"', $peek, $start);
    $count = $peek ? 1 : 0;
    $sb = '';
    $reading = true;
    while ($reading) {
      $c = $this->bufTrans->peek(1, $start + $count);
      switch ($c) {
        case '"':
          $reading = false;
          break;
        case '\\':
          $count++;
          $c = $this->bufTrans->peek(1, $start + $count);
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
              $sb .= chr(0x08);
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
              $sb .= hex2bin($this->bufTrans->peek(2, $start + $count));
              $count += 2;
              break;
            default:
              throw new TProtocolException(
                'TSimpleJSONProtocol: Expected Control Character, found 0x'.
                bin2hex($c),
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
      $this->trans_->readAll($count);
    }

    $this->expectChar('"', $peek, $start + $count);
    return Pair {$sb, $count + 1};
  }

  private function skipWhitespace(bool $peek = false, int $start = 0): int {
    if (!$peek) {
      $start = 0;
    }
    $count = 0;
    $reading = true;
    while ($reading) {
      $byte = $this->bufTrans->peek(1, $count + $start);
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
      $this->trans_->readAll($count);
    }

    return $count;
  }

  private function expectChar(
    string $char,
    bool $peek = false,
    int $start = 0,
  ): void {
    if (!$peek) {
      $start = 0;
    }
    $c = null;
    if ($peek) {
      $c = $this->bufTrans->peek(1, $start);
    } else {
      $c = $this->trans_->readAll(1);
    }

    if ($c !== $char) {
      throw new TProtocolException(
        'TSimpleJSONProtocol: Expected '.
        $char.
        ', but encountered 0x'.
        bin2hex($c),
      );
    }
  }

  private function guessFieldTypeBasedOnByte(string $byte): ?int {
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
        // These technically aren't allowed to start JSON floats, but are here for backwards compatibility
      case '+':
      case '.':
        return TType::DOUBLE;
      case '"':
        return TType::STRING;
      case ']':
      case '}':
        // We can get here with empty lists/maps, returning a dummy value
        return TType::STOP;
    }

    throw new TProtocolException(
      'TSimpleJSONProtocol: Unable to guess TType for character 0x'.
      bin2hex($byte),
    );
  }
}

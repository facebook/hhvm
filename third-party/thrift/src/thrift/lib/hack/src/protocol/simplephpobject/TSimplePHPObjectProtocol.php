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
 * @package thrift.protocol.simplephpobject
 */

class TSimplePHPObjectProtocol extends TProtocol {

  private Iterator<mixed> $top;
  private Vector<Iterator<mixed>> $stack;

  public function __construct(mixed $val) {
    $this->top = new ArrayIterator(array($val));
    $this->stack = Vector {$this->top};
    parent::__construct(new TNullTransport());
  }

  public function readMessageBegin(inout $name, inout $type, inout $seqid) {
    throw new TProtocolException('Not Supported');
  }

  public function readMessageEnd() {
    throw new TProtocolException('Not Supported');
  }

  public function readStructBegin(inout $name) {
    $name = null;
    $val = $this->top->current();
    $this->top->next();
    $itr = null;
    if ($val instanceof Set || $val instanceof ImmSet) {
      throw new TProtocolException(
        'Unsupported data structure for struct: '.gettype($val),
      );
    } else if ($val instanceof Vector) {
      if ($val->isEmpty()) {
        // Empty maps can be (de)serialized incorrectly as vectors
        $size = 0;
        $itr = $val->getIterator();
      } else {
        throw new TProtocolException(
          'Unsupported data structure for struct: '.gettype($val),
        );
      }
    } else if ($val instanceof ImmVector) {
      if ($val->isEmpty()) {
        // Empty maps can be (de)serialized incorrectly as vectors
        $size = 0;
        $itr = $val->getIterator();
      } else {
        throw new TProtocolException(
          'Unsupported data structure for struct: '.gettype($val),
        );
      }
    } else if ($val instanceof Map) {
      $itr = $val->getIterator();
    } else if ($val instanceof ImmMap) {
      $itr = $val->getIterator();
    } else if (is_array($val)) {
      $itr = new ArrayIterator($val);
    } else {
      $val = (array) $val;
      $itr = new ArrayIterator($val);
    }
    $itr = new TSimplePHPObjectProtocolKeyedIteratorWrapper($itr);
    $this->stack->add($itr);
    $this->top = $itr;
  }

  public function readStructEnd() {
    $this->stack->pop();
    $val = $this->stack->lastValue();
    invariant($val !== null, "Stack is empty, this shouldn't happen!");
    $this->top = $val;
  }

  public function readFieldBegin(
    inout $name,
    inout $fieldType,
    inout $fieldId,
  ) {
    $val = null;
    while ($val === null) {
      if (!$this->top->valid()) {
        $fieldType = TType::STOP;
        return;
      }

      $fieldId = null;
      $name = $this->top->current();
      $this->top->next();
      $val = $this->top->current();
      if ($val === null) {
        $this->top->next();
        continue;
      }

      $fieldType = $this->guessTypeForValue($val);
    }
  }

  public function readFieldEnd() {}

  public function readMapBegin(inout $keyType, inout $valType, inout $size) {
    $val = $this->top->current();
    $this->top->next();
    $itr = null;
    if ($val instanceof Set || $val instanceof ImmSet) {
      throw new TProtocolException(
        'Unsupported data structure for map: '.gettype($val),
      );
    } else if ($val instanceof Vector) {
      if ($val->isEmpty()) {
        // Empty maps can be (de)serialized incorrectly as vectors
        $size = 0;
        $itr = $val->getIterator();
      } else {
        throw new TProtocolException(
          'Unsupported data structure for map: '.gettype($val),
        );
      }
    } else if ($val instanceof ImmVector) {
      if ($val->isEmpty()) {
        // Empty maps can be (de)serialized incorrectly as vectors
        $size = 0;
        $itr = $val->getIterator();
      } else {
        throw new TProtocolException(
          'Unsupported data structure for map: '.gettype($val),
        );
      }
    } else if ($val instanceof Map) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if ($val instanceof ImmMap) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if (is_array($val)) {
      $itr = new ArrayIterator($val);
      $size = count($val);
    } else {
      $val = (array) $val;
      $itr = new ArrayIterator($val);
      $size = count($val);
    }
    $itr = new TSimplePHPObjectProtocolKeyedIteratorWrapper($itr);
    $this->stack->add($itr);
    $this->top = $itr;

    if ($itr->valid()) {
      $key_sample = $itr->current();
      $itr->next();
      $val_sample = $itr->current();
      $itr->rewind();

      $keyType = $this->guessTypeForValue($key_sample);
      $valType = $this->guessTypeForValue($val_sample);
    }
  }

  public function readMapEnd() {
    $this->stack->pop();
    $val = $this->stack->lastValue();
    invariant($val !== null, "Stack is empty, this shouldn't happen!");
    $this->top = $val;
  }

  public function readListBegin(&$elemType, &$size) {
    $val = $this->top->current();
    $this->top->next();
    $itr = null;
    if ($val instanceof Set || $val instanceof ImmSet) {
      throw new TProtocolException(
        'Unsupported data structure for list: '.gettype($val),
      );
    } else if ($val instanceof Map) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if ($val instanceof ImmMap) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if ($val instanceof Vector) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if ($val instanceof ImmVector) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if (is_array($val)) {
      $itr = new ArrayIterator($val);
      $size = count($val);
    } else {
      $val = (array) $val;
      $itr = new ArrayIterator($val);
      $size = count($val);
    }
    $this->stack->add($itr);
    $this->top = $itr;

    if ($itr->valid()) {
      $val_sample = $itr->current();
      $elemType = $this->guessTypeForValue($val_sample);
    }
  }

  public function readListEnd() {
    $this->stack->pop();
    $val = $this->stack->lastValue();
    invariant($val !== null, "Stack is empty, this shouldn't happen!");
    $this->top = $val;
  }

  public function readSetBegin(&$elemType, &$size) {
    $val = $this->top->current();
    $this->top->next();
    $itr = null;
    if ($val instanceof Map) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if ($val instanceof ImmMap) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if ($val instanceof Vector) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if ($val instanceof ImmVector) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if ($val instanceof Set) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if ($val instanceof ImmSet) {
      $itr = $val->getIterator();
      $size = $val->count();
    } else if (is_array($val)) {
      $itr = new ArrayIterator($val);
      $size = count($val);
    } else {
      $val = array_keys((array) $val);
      $itr = new ArrayIterator($val);
      $size = count($val);
    }
    $this->stack->add($itr);
    $this->top = $itr;

    if ($itr->valid()) {
      $val_sample = $itr->current();
      $elemType = $this->guessTypeForValue($val_sample);
    }
  }

  public function readSetEnd() {
    $this->stack->pop();
    $val = $this->stack->lastValue();
    invariant($val !== null, "Stack is empty, this shouldn't happen!");
    $this->top = $val;
  }

  public function readBool(&$value) {
    $value = (bool) $this->top->current();
    $this->top->next();
  }

  public function readByte(&$value) {
    $value = (int) $this->top->current();
    $this->top->next();
    if ($value < -0x80 || $value > 0x7F) {
      throw new TProtocolException('Value is outside of valid range');
    }
  }

  public function readI16(&$value) {
    $value = (int) $this->top->current();
    $this->top->next();
    if ($value < -0x8000 || $value > 0x7FFF) {
      throw new TProtocolException('Value is outside of valid range');
    }
  }

  public function readI32(&$value) {
    $value = (int) $this->top->current();
    $this->top->next();
    if ($value < -0x80000000 || $value > 0x7FFFFFFF) {
      throw new TProtocolException('Value is outside of valid range');
    }
  }

  public function readI64(&$value) {
    $value = (int) $this->top->current();
    $this->top->next();
    if ($value < (-0x80000000 << 32) ||
        $value > (0x7FFFFFFF << 32 | 0xFFFFFFFF)) {
      throw new TProtocolException('Value is outside of valid range');
    }
  }

  public function readDouble(&$value) {
    $value = (float) $this->top->current();
    $this->top->next();
  }

  public function readFloat(&$value) {
    $value = (float) $this->top->current();
    $this->top->next();
  }

  public function readString(&$value) {
    $value = (string) $this->top->current();
    $this->top->next();
  }

  public function writeMessageBegin($name, $type, $seqid) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeMessageEnd() {
    throw new TProtocolException('Not Implemented');
  }

  public function writeStructBegin($name) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeStructEnd() {
    throw new TProtocolException('Not Implemented');
  }

  public function writeFieldBegin($fieldName, $fieldType, $fieldId) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeFieldEnd() {
    throw new TProtocolException('Not Implemented');
  }

  public function writeFieldStop() {
    throw new TProtocolException('Not Implemented');
  }

  public function writeMapBegin($keyType, $valType, $size) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeMapEnd() {
    throw new TProtocolException('Not Implemented');
  }

  public function writeListBegin($elemType, $size) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeListEnd() {
    throw new TProtocolException('Not Implemented');
  }

  public function writeSetBegin($elemType, $size) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeSetEnd() {
    throw new TProtocolException('Not Implemented');
  }

  public function writeBool($value) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeByte($value) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeI16($value) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeI32($value) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeI64($value) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeDouble($value) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeFloat($value) {
    throw new TProtocolException('Not Implemented');
  }

  public function writeString($value) {
    throw new TProtocolException('Not Implemented');
  }

  private function guessTypeForValue(mixed $val): int {
    if (is_bool($val)) {
      return TType::BOOL;
    } else if (is_int($val)) {
      return TType::I64;
    } else if (is_float($val)) {
      return TType::DOUBLE;
    } else if (is_string($val)) {
      return TType::STRING;
    } else if (is_object($val) || is_array($val) || $val instanceof Map) {
      return TType::STRUCT;
    } else if ($val instanceof Iterable) {
      return TType::LST;
    }

    throw new TProtocolException(
      'Unable to guess thrift type for '.gettype($val),
    );
  }
}

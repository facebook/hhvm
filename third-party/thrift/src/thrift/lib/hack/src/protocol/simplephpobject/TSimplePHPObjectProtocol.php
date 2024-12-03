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

<<Oncalls('thrift')>> // @oss-disable
class TSimplePHPObjectProtocol extends TProtocol {
  const ctx CReadWriteDefault = [write_props];

  // 'copy-on-write iterator'. tuple tracks (position, iterable).
  const type TCOWIterator = (int, vec<mixed>);
  private vec<this::TCOWIterator> $stack;
  private int $stackTopIdx;

  public function __construct(mixed $val)[] {
    $this->stack = vec[tuple(0, vec[$val])];
    $this->stackTopIdx = 0;
    parent::__construct(new TNullTransport());
  }

  <<__Override>>
  public function readMessageBegin(
    inout string $_name,
    inout int $_type,
    inout int $_seqid,
  )[]: int {
    throw new TProtocolException('Not Supported');
  }

  <<__Override>>
  public function readMessageEnd()[]: int {
    throw new TProtocolException('Not Supported');
  }

  public static function toThriftObject<T as IThriftStruct>(
    mixed $simple_object,
    T $thrift_object,
    int $options = 0,
  )[write_props]: T {
    $protocol = new TSimplePHPObjectProtocol($simple_object);
    $protocol->setOptions($options);
    $thrift_object->read($protocol);
    return $thrift_object;
  }

  <<__Override>>
  public function readStructBegin(inout ?string $name)[write_props]: int {
    $name = null;
    $val = $this->stackTopIterCurrent() ?? vec[];
    $this->stackTopIterNext();
    if ($val is ConstSet<_>) {
      throw new TProtocolException(
        'Unsupported data structure for struct: '.PHP\gettype($val),
      );
    } else if ($val is ConstVector<_>) {
      if (!C\is_empty($val)) {
        throw new TProtocolException(
          'Unsupported data structure for struct: '.PHP\gettype($val),
        );
      }
      $val = vec[];
    } else if (!($val is KeyedContainer<_, _>)) {
      // @lint-ignore PHPISM_ERROR
      $val = PHPism_FIXME::arrayCast($val);
    }
    $this->stackPush(self::flattenMap($val));
    return 0;
  }

  <<__Override>>
  public function readStructEnd()[write_props]: int {
    $this->stackPopBack();
    return 0;
  }

  <<__Override>>
  public function readFieldBegin(
    inout ?string $name,
    inout ?TType $field_type,
    inout ?int $field_id,
  )[write_props]: int {
    $val = null;
    while ($val === null) {
      if (!$this->stackTopIterValid()) {
        $field_type = TType::STOP;
        return 0;
      }

      $field_id = null;
      $name =
        // It's expected to use it only for keys, which seem to be strings or
        // alike.
        PHPism_FIXME::stringishCastPure_UNSAFE($this->stackTopIterCurrent());
      $this->stackTopIterNext();
      $val = $this->stackTopIterCurrent();
      if ($val === null) {
        $this->stackTopIterNext();
        continue;
      }

      $field_type = self::guessTypeForValue($val);
    }
    return 0;
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
  )[write_props]: int {
    $val = $this->stackTopIterCurrent() ?? vec[];
    $this->stackTopIterNext();
    if ($val is ConstSet<_>) {
      throw new TProtocolException(
        'Unsupported data structure for map: '.PHP\gettype($val),
      );
    } else if ($val is ConstVector<_>) {
      if (!C\is_empty($val)) {
        throw new TProtocolException(
          'Unsupported data structure for map: '.PHP\gettype($val),
        );
      }
      // Empty maps can be (de)serialized incorrectly as vectors.
      // We cannot correctly handle a Vector with values when reading a
      // Map, but we can infer an empty vector as an empty map.
      $val = vec[];
    } else if (!($val is KeyedContainer<_, _>)) {
      // @lint-ignore PHPISM_ERROR
      $val = PHPism_FIXME::arrayCast($val);
    }
    // @lint-ignore UNUSED_VARIABLE linter is confused by an inout variable
    $size = C\count($val);
    $this->stackPush(self::flattenMap($val));

    if ($this->stackTopIterValid()) {
      $key_sample = $this->stackTopIterCurrent();
      $this->stackTopIterNext();
      $val_sample = $this->stackTopIterCurrent();
      $this->stackTopIterRewind();

      $key_type = self::guessTypeForValue($key_sample);
      $val_type = self::guessTypeForValue($val_sample);
    }
    return 0;
  }

  <<__Override>>
  public function readMapEnd()[write_props]: int {
    $this->stackPopBack();
    return 0;
  }

  <<__Override>>
  public function readListBegin(
    inout ?TType $elem_type,
    inout ?int $size,
  )[write_props]: int {
    $val = $this->stackTopIterCurrent() ?? vec[];
    $this->stackTopIterNext();
    if ($val is ConstSet<_> || $val is string) {
      throw new TProtocolException(
        'Unsupported data structure for list: '.PHP\gettype($val),
      );
    }
    if (!($val is KeyedContainer<_, _>)) {
      // @lint-ignore PHPISM_ERROR
      $val = vec(PHPism_FIXME::arrayCast($val));
    }
    // @lint-ignore UNUSED_VARIABLE linter is confused by an inout variable
    $size = C\count($val);
    $this->stackPush(vec($val));

    if ($this->stackTopIterValid()) {
      $val_sample = $this->stackTopIterCurrent();
      $elem_type = self::guessTypeForValue($val_sample);
    }
    return 0;
  }

  <<__Override>>
  public function readListEnd()[write_props]: int {
    $this->stackPopBack();
    return 0;
  }

  <<__Override>>
  public function readSetBegin(
    inout ?TType $elem_type,
    inout ?int $size,
  )[write_props]: int {
    $val = $this->stackTopIterCurrent() ?? vec[];
    $this->stackTopIterNext();
    if ($val is Container<_>) {
      $val = vec($val);
    } else {
      // @lint-ignore PHPISM_ERROR
      $val = Vec\keys(PHPism_FIXME::arrayCast($val));
    }
    // @lint-ignore UNUSED_VARIABLE linter is confused by an inout variable
    $size = C\count($val);
    $this->stackPush($val);

    if ($this->stackTopIterValid()) {
      $val_sample = $this->stackTopIterCurrent();
      $elem_type = self::guessTypeForValue($val_sample);
    }
    return 0;
  }

  <<__Override>>
  public function readSetEnd()[write_props]: int {
    $this->stackPopBack();
    return 0;
  }

  <<__Override>>
  public function readBool(inout bool $value)[write_props]: int {
    $value = (bool)$this->stackTopIterCurrent();
    $this->stackTopIterNext();
    return 0;
  }

  <<__Override>>
  public function readByte(inout int $value)[write_props]: int {
    $value = (int)$this->stackTopIterCurrent();
    $this->stackTopIterNext();
    if ($value < -0x80 || $value > 0x7F) {
      throw new TProtocolException('Value is outside of valid range');
    }
    return 0;
  }

  <<__Override>>
  public function readI16(inout int $value)[write_props]: int {
    $value = (int)$this->stackTopIterCurrent();
    $this->stackTopIterNext();
    if ($value < -0x8000 || $value > 0x7FFF) {
      throw new TProtocolException('Value is outside of valid range');
    }
    return 0;
  }

  <<__Override>>
  public function readI32(inout int $value)[write_props]: int {
    $value = (int)$this->stackTopIterCurrent();
    $this->stackTopIterNext();
    if ($value < -0x80000000 || $value > 0x7FFFFFFF) {
      throw new TProtocolException('Value is outside of valid range');
    }
    return 0;
  }

  <<__Override>>
  public function readI64(inout int $value)[write_props]: int {
    $value = (int)$this->stackTopIterCurrent();
    $this->stackTopIterNext();
    if (
      $value < (-0x80000000 << 32) || $value > (0x7FFFFFFF << 32 | 0xFFFFFFFF)
    ) {
      throw new TProtocolException('Value is outside of valid range');
    }
    return 0;
  }

  <<__Override>>
  public function readDouble(inout float $value)[write_props]: int {
    $value = (float)$this->stackTopIterCurrent();
    $this->stackTopIterNext();
    return 0;
  }

  <<__Override>>
  public function readFloat(inout float $value)[write_props]: int {
    $value = (float)$this->stackTopIterCurrent();
    $this->stackTopIterNext();
    return 0;
  }

  <<__Override>>
  public function readString(inout string $value)[write_props]: int {
    $value = (string)$this->stackTopIterCurrent();
    $this->stackTopIterNext();
    return 0;
  }

  <<__Override>>
  public function writeMessageBegin(
    string $_name,
    TMessageType $_type,
    int $_seqid,
  )[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeMessageEnd()[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeStructBegin(string $_name)[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeStructEnd()[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeFieldBegin(
    string $_field_name,
    TType $_field_type,
    int $_field_id,
  )[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeFieldEnd()[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeFieldStop()[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeMapBegin(
    TType $_key_type,
    TType $_val_type,
    int $_size,
  )[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeMapEnd()[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeListBegin(TType $_elem_type, int $_size)[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeListEnd()[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeSetBegin(TType $_elem_ype, int $_size)[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeSetEnd()[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeBool(bool $_value)[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeByte(int $_value)[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeI16(int $_value)[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeI32(int $_value)[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeI64(int $_value)[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeDouble(float $_value)[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeFloat(float $_value)[]: int {
    throw new TProtocolException('Not Implemented');
  }

  <<__Override>>
  public function writeString(string $_value)[]: int {
    throw new TProtocolException('Not Implemented');
  }

  /**
   * Returns the index of the topmost element in the stack.
   */
  private function stackTopIdx()[]: int {
    return $this->stackTopIdx;
  }

  /**
   * Returns true if the position of the iterator at the top of the stack is
   * valid, or false if the position is out of bounds.
   */
  private function stackTopIterValid()[]: bool {
    list($top_pos, $top_iter) = C\last($this->stack) as nonnull;
    return $top_pos >= 0 && $top_pos < C\count($top_iter);
  }

  /**
   * Returns the current element for the iterator at the top of the stack.
   */
  private function stackTopIterCurrent()[]: mixed {
    list($top_pos, $top_iter) = C\last($this->stack) as nonnull;
    return $top_iter[$top_pos];
  }

  /**
   * Moves the position for the iterator at the top of the stack forward.
   */
  private function stackTopIterNext()[write_props]: void {
    $this->stack[$this->stackTopIdx()][0]++;
  }

  /**
   * Moves the position for the iterator at the top of the stack backward.
   */
  private function stackTopIterRewind()[write_props]: void {
    $this->stack[$this->stackTopIdx()][0]--;
  }

  /**
   * Turns an iterable into a COW iterator and pushes it onto the stack.
   */
  private function stackPush(vec<mixed> $iterable)[write_props]: void {
    $this->stack[] = tuple(0, $iterable);
    $this->stackTopIdx++;
  }

  /**
   * Pops the iterator at the top of the stack.
   */
  private function stackPopBack()[write_props]: void {
    /* HH_FIXME[4135] Exposed by banning unset and isset in partial mode */
    unset($this->stack[$this->stackTopIdx()]);
    $this->stackTopIdx--;
    invariant(
      !C\is_empty($this->stack),
      'Stack is empty, this shouldn\'t happen!',
    );
  }

  /**
   * Flattens the keys and values of a map sequentially into a vec.
   *
   * e.g. dict['foo' => 123, 'bar' => 456] -> vec['foo', 123, 'bar', 456]
   */
  private static function flattenMap(
    KeyedContainer<arraykey, mixed> $map,
  )[]: vec<mixed> {
    $res = vec[];
    foreach ($map as $key => $value) {
      $res[] = $key;
      $res[] = $value;
    }
    return $res;
  }

  private static function guessTypeForValue(mixed $val)[]: TType {
    if ($val is bool) {
      return TType::BOOL;
    } else if ($val is int) {
      return TType::I64;
    } else if ($val is float) {
      return TType::DOUBLE;
    } else if ($val is string) {
      return TType::STRING;
    } else if (
      PHP\is_object($val) || ($val is dict<_, _>) || $val is Map<_, _>
    ) {
      return TType::STRUCT;
    } else if ($val is Iterable<_> || ($val is vec<_>)) {
      return TType::LST;
    } else if ($val is keyset<_>) {
      return TType::SET;
    }

    throw new TProtocolException(
      'Unable to guess thrift type for '.PHP\gettype($val),
    );
  }
}

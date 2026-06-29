<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
/**
 * Map used by generated thrift code for object-key maps, where thrift key
 * identity cannot be represented directly by Hack arraykey equality. Keys are
 * stored internally as canonical serialized thrift bytes for lookup.
 *
 * Iteration order is not defined. Do not depend on
 * traversal, `getKeys()`, `getValues()`, or `toShape()` returning a stable
 * order.
 */
final class ThriftMap<reify TKey, reify TValue>
  implements Countable, KeyedIterable<TKey, TValue> {
  use StrictKeyedIterable<TKey, TValue>;

  // Entries are keyed by canonical compact-protocol bytes for the thrift key.
  // Lookups are normal dict lookups; traversal order is not defined.
  private dict<string, TValue> $entries = dict[];
  private TType $keyType;
  private ThriftStructTypes::TGenericSpec $keySpec;

  public function __construct(
    TType $key_type,
    ThriftStructTypes::TGenericSpec $key_spec,
    dict<string, TValue> $entries = dict[],
  )[] {
    $this->keyType = $key_type;
    $this->keySpec = $key_spec;
    $this->entries = $entries;
  }

  public static function forStruct<reify TK as IThriftStruct, reify TV>(
  )[]: ThriftMap<TK, TV> {
    return new ThriftMap<TK, TV>(
      TType::STRUCT,
      shape(
        'type' => TType::STRUCT,
        'class' => HH\ReifiedGenerics\get_class_from_type<TK>(),
      ),
    );
  }

  public static function forBool<reify TV>()[]: ThriftMap<bool, TV> {
    return new ThriftMap<bool, TV>(TType::BOOL, shape('type' => TType::BOOL));
  }

  public static function forFloat<reify TV>()[]: ThriftMap<float, TV> {
    return
      new ThriftMap<float, TV>(TType::DOUBLE, shape('type' => TType::DOUBLE));
  }

  private function serializeKey(TKey $key)[write_props]: string {
    return ThriftKeySerializer::serialize($key, $this->keyType, $this->keySpec);
  }

  private function deserializeKey(string $serialized)[write_props]: TKey {
    return HH\FIXME\UNSAFE_CAST<mixed, TKey>(
      ThriftKeySerializer::deserialize(
        $serialized,
        $this->keyType,
        $this->keySpec,
      ),
      'Thrift key deserialization preserves the map key type',
    );
  }

  private function serializedKeys()[]: vec<string> {
    return Vec\keys($this->entries);
  }

  public function set(TKey $key, TValue $value)[write_props]: this {
    $this->entries[$this->serializeKey($key)] = $value;
    return $this;
  }

  public function get(TKey $key)[write_props]: ?TValue {
    $serialized = $this->serializeKey($key);
    return C\contains_key($this->entries, $serialized)
      ? $this->entries[$serialized]
      : null;
  }

  public function at(TKey $key)[write_props]: TValue {
    $serialized = $this->serializeKey($key);
    invariant(
      C\contains_key($this->entries, $serialized),
      'Key not found in ThriftMap',
    );
    return $this->entries[$serialized];
  }

  public function contains(TKey $key)[write_props]: bool {
    return C\contains_key($this->entries, $this->serializeKey($key));
  }

  public function remove(TKey $key)[write_props]: this {
    $serialized = $this->serializeKey($key);
    unset($this->entries[$serialized]);
    return $this;
  }

  public function count()[]: int {
    return C\count($this->entries);
  }

  public function isEmpty()[]: bool {
    return C\is_empty($this->entries);
  }

  public function getIterator()[write_props]: KeyedIterator<TKey, TValue> {
    return new ThriftMapIterator(
      $this->serializedKeys(),
      $this->entries,
      $this->keyType,
      $this->keySpec,
    );
  }

  /**
   * Returns keys in an undefined order. Do not correlate this vector with
   * `getValues()`; request pairs via iteration or `toShape()` instead.
   */
  public function getKeys()[write_props]: vec<TKey> {
    return Vec\map(
      $this->serializedKeys(),
      $serialized ==> $this->deserializeKey($serialized),
    );
  }

  /**
   * Returns values in an undefined order. Do not correlate this vector
   * with `getKeys()`; request pairs via iteration or `toShape()` instead.
   */
  public function getValues()[]: vec<TValue> {
    return Vec\map(
      $this->serializedKeys(),
      $serialized ==> $this->entries[$serialized],
    );
  }

  public function toShape()[write_props]: vec<(TKey, TValue)> {
    return Vec\map(
      $this->serializedKeys(),
      $serialized ==>
        tuple($this->deserializeKey($serialized), $this->entries[$serialized]),
    );
  }

  public function copy()[]: ThriftMap<TKey, TValue> {
    return new ThriftMap<TKey, TValue>(
      $this->keyType,
      $this->keySpec,
      $this->entries,
    );
  }

  public static function fromShape<reify TK, reify TV>(
    vec<(TK, TV)> $entries,
    TType $key_type,
    ThriftStructTypes::TGenericSpec $key_spec,
  )[write_props]: ThriftMap<TK, TV> {
    $map = new ThriftMap<TK, TV>($key_type, $key_spec);
    foreach ($entries as list($key, $value)) {
      $serialized = $map->serializeKey($key);
      invariant(
        !C\contains_key($map->entries, $serialized),
        'Duplicate key in ThriftMap::fromShape',
      );
      $map->entries[$serialized] = $value;
    }
    return $map;
  }
}

final class ThriftMapIterator<TKey, TValue>
  implements KeyedIterator<TKey, TValue> {

  private int $idx = 0;
  private vec<(TKey, TValue)> $entries;

  public function __construct(
    vec<string> $serialized_keys,
    dict<string, TValue> $entries,
    TType $key_type,
    ThriftStructTypes::TGenericSpec $key_spec,
  )[write_props] {
    $this->entries = Vec\map(
      $serialized_keys,
      $serialized ==> tuple(
        HH\FIXME\UNSAFE_CAST<mixed, TKey>(
          ThriftKeySerializer::deserialize($serialized, $key_type, $key_spec),
          'Thrift key deserialization preserves the map key type',
        ),
        $entries[$serialized],
      ),
    );
  }

  public function current(): TValue {
    return $this->entries[$this->idx][1];
  }

  public function key(): TKey {
    return $this->entries[$this->idx][0];
  }

  public function next(): void {
    $this->idx++;
  }

  public function rewind(): void {
    $this->idx = 0;
  }

  public function valid(): bool {
    return C\contains_key($this->entries, $this->idx);
  }
}

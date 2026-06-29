<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
/**
 * Set used by generated thrift code for object-key sets, where thrift element
 * identity cannot be represented directly by Hack arraykey equality. Elements
 * are stored internally as canonical serialized thrift bytes for lookup.
 *
 * Iteration order is not defined. Do not depend on
 * traversal or `toVec()` returning a stable order.
 */
final class ThriftSet<TElem> implements Countable, Iterable<TElem> {
  use StrictIterable<TElem>;

  // Entries are canonical compact-protocol bytes for the thrift element.
  // Lookups are normal keyset membership checks; traversal order is undefined.
  private keyset<string> $entries = keyset[];
  private TType $elemType;
  private ThriftStructTypes::TGenericSpec $elemSpec;

  public function __construct(
    TType $elem_type,
    ThriftStructTypes::TGenericSpec $elem_spec,
    keyset<string> $entries = keyset[],
  )[] {
    $this->elemType = $elem_type;
    $this->elemSpec = $elem_spec;
    $this->entries = $entries;
  }

  public static function forStruct<reify TE as IThriftStruct>(
  )[]: ThriftSet<TE> {
    return new ThriftSet(
      TType::STRUCT,
      shape(
        'type' => TType::STRUCT,
        'class' => HH\ReifiedGenerics\get_class_from_type<TE>(),
      ),
    );
  }

  public static function forBool()[]: ThriftSet<bool> {
    return new ThriftSet(TType::BOOL, shape('type' => TType::BOOL));
  }

  public static function forFloat()[]: ThriftSet<float> {
    return new ThriftSet(TType::DOUBLE, shape('type' => TType::DOUBLE));
  }

  private function serializeElement(TElem $element)[write_props]: string {
    return ThriftKeySerializer::serialize(
      $element,
      $this->elemType,
      $this->elemSpec,
    );
  }

  private function deserializeElement(string $serialized)[write_props]: TElem {
    return HH\FIXME\UNSAFE_CAST<mixed, TElem>(
      ThriftKeySerializer::deserialize(
        $serialized,
        $this->elemType,
        $this->elemSpec,
      ),
      'Thrift key deserialization preserves the set element type',
    );
  }

  private function serializedEntries()[]: vec<string> {
    return vec($this->entries);
  }

  public function add(TElem $element)[write_props]: this {
    $this->entries[] = $this->serializeElement($element);
    return $this;
  }

  public function contains(TElem $element)[write_props]: bool {
    return C\contains($this->entries, $this->serializeElement($element));
  }

  public function remove(TElem $element)[write_props]: this {
    $serialized = $this->serializeElement($element);
    $this->entries = Keyset\diff($this->entries, keyset[$serialized]);
    return $this;
  }

  public function count()[]: int {
    return C\count($this->entries);
  }

  public function isEmpty()[]: bool {
    return C\is_empty($this->entries);
  }

  public function getIterator()[write_props]: Iterator<TElem> {
    foreach ($this->serializedEntries() as $entry) {
      yield $this->deserializeElement($entry);
    }
  }

  public function toVec()[write_props]: vec<TElem> {
    return Vec\map(
      $this->serializedEntries(),
      $entry ==> $this->deserializeElement($entry),
    );
  }

  public function copy()[]: ThriftSet<TElem> {
    return
      new ThriftSet<TElem>($this->elemType, $this->elemSpec, $this->entries);
  }

  public static function fromVec<TE>(
    vec<TE> $elements,
    TType $elem_type,
    ThriftStructTypes::TGenericSpec $elem_spec,
  )[write_props]: ThriftSet<TE> {
    $set = new ThriftSet<TE>($elem_type, $elem_spec);
    foreach ($elements as $element) {
      $serialized = $set->serializeElement($element);
      invariant(
        !C\contains($set->entries, $serialized),
        'Duplicate element in ThriftSet::fromVec',
      );
      $set->entries[] = $serialized;
    }
    return $set;
  }
}

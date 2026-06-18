<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
abstract final class ThriftKeySerializer {

  // Returns canonical compact-protocol value bytes for key identity. These are
  // not a full thrift message; for bools they use the same payload byte compact
  // protocol writes for bool collection elements and map keys.
  public static function serialize(
    mixed $value,
    TType $type,
    ThriftStructTypes::TGenericSpec $spec,
  )[write_props]: string {
    return HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
      ()[defaults] ==> {
        $transport = new TMemoryBuffer();
        $protocol = new TCompactProtocolAccelerated($transport);
        if ($type === TType::BOOL) {
          // Compact bool values are normally encoded through field or container
          // state. Key payloads are standalone bytes, so write the collection
          // bool encoding directly.
          $thrift_value =
            ThriftSerializationHelper::unwrapApplyAdapter($value, $spec);
          $protocol->writeByte(
            HH\legacy_is_truthy($thrift_value)
              ? TCompactProtocolBase::COMPACT_TRUE
              : TCompactProtocolBase::COMPACT_FALSE,
          );
        } else {
          self::writeHackValueInCurrentContext($protocol, $value, $type, $spec);
        }
        return $transport->getBuffer();
      },
      'Thrift key serialization writes to an in-memory compact protocol.',
    );
  }

  public static function deserialize(
    string $bytes,
    TType $type,
    ThriftStructTypes::TGenericSpec $spec,
  )[write_props]: mixed {
    return HH\Coeffects\fb\backdoor_from_write_props__DO_NOT_USE(
      ()[defaults] ==> {
        if ($type === TType::BOOL) {
          $transport = new TMemoryBuffer();
          $transport->write($bytes);
          $protocol = new TCompactProtocolAccelerated($transport);
          $byte = 0;
          $protocol->readByte(inout $byte);
          return ThriftSerializationHelper::applyAdapterFromThrift(
            $byte === TCompactProtocolBase::COMPACT_TRUE,
            $spec,
          );
        }
        return TCompactSerializer::deserializeData($bytes, $spec);
      },
      'Thrift key deserialization reads from in-memory compact key bytes.',
    );
  }

  // Use after writeSetBegin/writeMapBegin when the element/key bytes were
  // already produced by serialize().
  private static function writeSerializedKeyPayload(
    TCompactProtocolAccelerated $protocol,
    string $payload,
  ): void {
    $protocol->getTransport()->write($payload);
  }

  // Use for Hack-form values read from container APIs. This applies adapter and
  // wrapper specs before writing into the caller's compact-protocol context.
  private static function writeHackValueInCurrentContext(
    TCompactProtocolAccelerated $protocol,
    mixed $value,
    TType $type,
    ThriftStructTypes::TGenericSpec $spec,
  ): void {
    $value = ThriftSerializationHelper::unwrapApplyAdapter($value, $spec);
    self::writeThriftValueInCurrentContext($protocol, $value, $type, $spec);
  }

  // Use only after the caller has already converted the value to thrift form,
  // such as struct fields where terse/default checks must see that form.
  private static function writeThriftValueInCurrentContext(
    TCompactProtocolAccelerated $protocol,
    mixed $value,
    TType $type,
    ThriftStructTypes::TGenericSpec $spec,
  ): void {
    switch ($type) {
      case TType::BOOL:
        $protocol->writeBool(HH\legacy_is_truthy($value));
        return;
      case TType::BYTE:
        $protocol->writeByte((int)$value);
        return;
      case TType::I16:
        $protocol->writeI16((int)$value);
        return;
      case TType::I32:
        $protocol->writeI32((int)$value);
        return;
      case TType::I64:
        $protocol->writeI64((int)$value);
        return;
      case TType::DOUBLE:
        $protocol->writeDouble((float)$value);
        return;
      case TType::FLOAT:
        $protocol->writeFloat((float)$value);
        return;
      case TType::STRING:
      case TType::UTF8:
      case TType::UTF16:
        $str = (string)$value;
        if (Shapes::idx($spec, 'is_binary') ?? false) {
          $protocol->writeBinary($str);
        } else {
          $protocol->writeString($str);
        }
        return;
      case TType::STRUCT:
        self::writeStruct($protocol, $value as IThriftStruct);
        return;
      case TType::LST:
        self::writeList($protocol, $value, $spec);
        return;
      case TType::SET:
        self::writeSet($protocol, $value, $spec);
        return;
      case TType::MAP:
        self::writeMap($protocol, $value, $spec);
        return;
      default:
        invariant_violation(
          'ThriftKeySerializer: unsupported TType %d',
          (int)$type,
        );
    }
  }

  private static function writeStruct(
    TCompactProtocolAccelerated $protocol,
    IThriftStruct $object,
  ): void {
    $protocol->writeStructBegin($object->getName());
    foreach (Vec\sort(Vec\keys($object::SPEC)) as $field_id) {
      $fspec = $object::SPEC[$field_id];
      $field_value = ThriftSerializationHelper::unwrapApplyAdapter(
        ThriftSerializationHelper::readStructField($object, $fspec),
        $fspec,
      );
      if (ThriftSerializationHelper::isFieldEmpty($field_value, $fspec)) {
        continue;
      }

      $field_type = $fspec['type'];
      $protocol->writeFieldBegin($fspec['var'], $field_type, $field_id);
      self::writeThriftValueInCurrentContext(
        $protocol,
        $field_value,
        $field_type,
        $fspec,
      );
      $protocol->writeFieldEnd();
    }
    $protocol->writeFieldStop();
    $protocol->writeStructEnd();
  }

  private static function writeList(
    TCompactProtocolAccelerated $protocol,
    mixed $object,
    ThriftStructTypes::TGenericSpec $spec,
  ): void {
    $etype = Shapes::at($spec, 'etype');
    $espec = Shapes::at($spec, 'elem');
    $elements = vec($object as Traversable<_>);
    $protocol->writeListBegin($etype, C\count($elements));
    foreach ($elements as $element) {
      self::writeHackValueInCurrentContext($protocol, $element, $etype, $espec);
    }
    $protocol->writeListEnd();
  }

  private static function writeSet(
    TCompactProtocolAccelerated $protocol,
    mixed $object,
    ThriftStructTypes::TGenericSpec $spec,
  ): void {
    $etype = Shapes::at($spec, 'etype');
    $espec = Shapes::at($spec, 'elem');
    $elements = Shapes::idx($spec, 'format') === 'array'
      ? Vec\keys($object as KeyedTraversable<_, _>)
      : vec($object as Traversable<_>);
    $sorted = Vec\sort(
      Vec\map(
        $elements,
        $element ==> self::serialize($element, $etype, $espec),
      ),
      ($a, $b) ==> Str\compare($a, $b),
    );

    $protocol->writeSetBegin($etype, C\count($sorted));
    foreach ($sorted as $serialized_element) {
      self::writeSerializedKeyPayload($protocol, $serialized_element);
    }
    $protocol->writeSetEnd();
  }

  private static function writeMap(
    TCompactProtocolAccelerated $protocol,
    mixed $object,
    ThriftStructTypes::TGenericSpec $spec,
  ): void {
    $ktype = Shapes::at($spec, 'ktype');
    $vtype = Shapes::at($spec, 'vtype');
    $kspec = Shapes::at($spec, 'key');
    $vspec = Shapes::at($spec, 'val');
    $sorted = Vec\sort(
      Vec\map_with_key(
        $object as KeyedTraversable<_, _>,
        ($key, $value) ==> tuple(self::serialize($key, $ktype, $kspec), $value),
      ),
      ($a, $b) ==> Str\compare($a[0], $b[0]),
    );

    $protocol->writeMapBegin($ktype, $vtype, C\count($sorted));
    foreach ($sorted as list($serialized_key, $value)) {
      self::writeSerializedKeyPayload($protocol, $serialized_key);
      self::writeHackValueInCurrentContext($protocol, $value, $vtype, $vspec);
    }
    $protocol->writeMapEnd();
  }
}

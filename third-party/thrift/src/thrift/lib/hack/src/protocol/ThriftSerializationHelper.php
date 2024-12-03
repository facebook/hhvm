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
 * Generic Protocol Reader and Writer
 */
<<Oncalls('thrift')>> // @oss-disable
abstract final class ThriftSerializationHelper {

  public static function readStruct(
    TProtocol $protocol,
    IThriftStruct $object,
  )[$protocol::CReadWriteDefault, write_props]: int {
    $field_name = '';
    $field_type = null;
    $field_id = 0;

    $tspec = $object::SPEC;
    $xfer = $protocol->readStructBegin(inout $field_name);
    while (true) {
      $xfer += $protocol->readFieldBegin(
        inout $field_name,
        inout $field_type,
        inout $field_id,
      );

      // Break once we reach the end of the struct.
      if ($field_type === TType::STOP) {
        break;
      }

      if ($field_id !== null) {
        // Compact, Binary and JSON:
        // These protocols only encode the field id.

        // Versioning:
        // Skip type if there is a mismatch between the tspec and the
        // type we obtain from the buffer.
        if (
          !C\contains_key($tspec, $field_id) ||
          $field_type !== $tspec[$field_id]['type']
        ) {
          $xfer +=
            $protocol->skip(nullthrows($field_type, 'Got unexpected null'));
          $xfer += $protocol->readFieldEnd();
          continue;
        }

        // This uses the TSPEC to find the field name.
        $field_name = $tspec[$field_id]['var'];
      } else {
        // SimpleJSON:
        // This protocol only encodes the field name.

        // Versioning:
        // Skip type if there is a mismatch between the tspec and the
        // type that we obtain from the buffer.
        if (!PHP\array_key_exists($field_name, $object::FIELDMAP)) {
          $xfer +=
            $protocol->skip(nullthrows($field_type, 'Got unexpected null'));
          $xfer += $protocol->readFieldEnd();
          continue;
        }

        // This uses the TFIELDMAP to find the field id.
        $field_id =
          $object::FIELDMAP[nullthrows($field_name, 'Got unexpected null')];
      }

      $tmp = null;
      $has_type_wrapper = false;
      $xfer += self::readStructHelper(
        $protocol,
        $tspec[$field_id]['type'],
        inout $tmp,
        $tspec[$field_id],
        inout $has_type_wrapper,
      );
      $is_field_wrapped = Shapes::idx($tspec[$field_id], 'is_wrapped') ?? false;

      if ($has_type_wrapper && $field_name is nonnull) {
        $setter_method = "set_".$field_name."_DO_NOT_USE_THRIFT_INTERNAL";
        /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
        $object->$setter_method($tmp);
      } else if ($is_field_wrapped && $field_name is nonnull) {
        $getter_method = "get_".$field_name;
        /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
        HH\FIXME\UNSAFE_CAST<mixed,HH_FIXME\UNKNOWN_TYPE_FOR_CAST>($object->$getter_method())->setValue_DO_NOT_USE_THRIFT_INTERNAL($tmp);
      } else {
        /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
        $object->$field_name = $tmp;
      }
      $xfer += $protocol->readFieldEnd();
    }
    $xfer += $protocol->readStructEnd();
    return $xfer;
  }

  public static function readStructHelper(
    TProtocol $protocol,
    TType $field_type,
    inout mixed $object,
    ThriftStructTypes::TGenericSpec $tspec,
    inout bool $has_type_wrapper,
  )[$protocol::CReadWriteDefault, write_props]: int {
    $xfer = 0;
    switch ($field_type) {
      case TType::BOOL:
        $val = false;
        $xfer += $protocol->readBool(inout $val);
        $object = $val;
        break;
      case TType::BYTE:
        $val = 0;
        $xfer += $protocol->readByte(inout $val);
        $object = $val;
        break;
      case TType::I16:
        $val = 0;
        $xfer += $protocol->readI16(inout $val);
        $object = $val;
        break;
      case TType::I32:
        // Enums:
        // In Hack, enums are encoded as I32s.
        // This looks into the tspec to distinguish the two of them.
        // Optimization opportunity: Add a TType of enum and encode that to
        // the tspec to avoid this if statement.
        $val = 0;
        $xfer += $protocol->readI32(inout $val);
        if (Shapes::keyExists($tspec, 'enum')) {
          $enum_class = ArgAssert::isEnumname(
            HH_FIXME::tryClassToClassname($tspec['enum']),
          );
          $object = HH\classname_to_class($enum_class) |> $$::coerce($val);
        } else {
          $object = $val;
        }
        break;
      case TType::I64:
        $val = 0;
        $xfer += $protocol->readI64(inout $val);
        $object = $val;
        break;
      case TType::DOUBLE:
        $val = 0.0;
        $xfer += $protocol->readDouble(inout $val);
        $object = $val;
        break;
      case TType::FLOAT:
        $val = 0.0;
        $xfer += $protocol->readFloat(inout $val);
        $object = $val;
        break;
      case TType::STRING:
        $val = '';
        $is_binary = Shapes::idx($tspec, 'is_binary') ?? false;
        if ($is_binary) {
          $xfer += $protocol->readBinary(inout $val);
        } else {
          $xfer += $protocol->readString(inout $val);
        }
        $object = $val;
        break;
      case TType::LST:
        $size = 0;
        $element_type = null;
        $xfer += $protocol->readListBegin(inout $element_type, inout $size);

        $list = vec[];
        $elem_spec = Shapes::at($tspec, 'elem');
        $has_type_wrapper = $has_type_wrapper ||
          (Shapes::idx($elem_spec, 'is_type_wrapped') ?? false);
        for ($i = 0; $size === null || $i < $size; ++$i) {
          if ($size === null && !$protocol->readListHasNext()) {
            break;
          }

          $list_element = null;
          $xfer += self::readStructHelper(
            $protocol,
            Shapes::at($tspec, 'etype'),
            inout $list_element,
            $elem_spec,
            inout $has_type_wrapper,
          );

          // If the element type is enum and the enum
          // does not exist, it will return a null.
          if ($list_element === null) {
            continue;
          }

          $list[] = $list_element;
        }

        // Convert collection to the correct format.
        if (Shapes::at($tspec, 'format') === 'harray') {
          $list = $list;
        } else if (Shapes::at($tspec, 'format') === 'collection') {
          $list = new Vector($list);
        } else { // format === 'array'
          $list = ($protocol->getOptions() & THRIFT_MARK_LEGACY_ARRAYS)
            ? HH\array_mark_legacy($list)
            : $list;
        }

        $object = $list;
        $xfer += $protocol->readListEnd();
        break;
      case TType::SET:
        $size = 0;
        $element_type = null;
        $xfer += $protocol->readSetBegin(inout $element_type, inout $size);

        $set = keyset[];
        for ($i = 0; $size === null || $i < $size; ++$i) {
          if ($size === null && !$protocol->readSetHasNext()) {
            break;
          }

          $set_element = null;
          $xfer += self::readStructHelper(
            $protocol,
            Shapes::at($tspec, 'etype'),
            inout $set_element,
            Shapes::at($tspec, 'elem'),
            inout $has_type_wrapper,
          );

          // If the element type is enum and the enum
          // does not exist, it will return a null.
          if ($set_element === null) {
            continue;
          }

          $set[] = HH\FIXME\UNSAFE_CAST<nonnull, arraykey>(
            $set_element,
            'Keyset value must be an arraykey',
          );
        }

        // Convert collection to the correct format.
        if (Shapes::at($tspec, 'format') === 'harray') {
          $set = $set;
        } else if (Shapes::at($tspec, 'format') === 'collection') {
          $set = new Set($set);
        } else { // format === 'array'
          // When using a set array(), we can't append in the normal way.
          // Therefore, we need to distinguish between the two types
          // before we add the element to the set.
          $tmp = ($protocol->getOptions() & THRIFT_MARK_LEGACY_ARRAYS)
            ? HH\array_mark_legacy(dict[])
            : dict[];
          foreach ($set as $set_element) {
            $tmp[PHPArrayism::preserveLegacyKey($set_element)] = true;
          }
          $set = $tmp;
        }

        $object = $set;
        $xfer += $protocol->readSetEnd();
        break;
      case TType::MAP:
        $size = 0;
        $key_type = null;
        $value_type = null;
        $xfer += $protocol->readMapBegin(
          inout $key_type,
          inout $value_type,
          inout $size,
        );

        $map = dict[];
        $val_spec = Shapes::at($tspec, 'val');
        $has_type_wrapper = $has_type_wrapper ||
          (Shapes::idx($val_spec, 'is_type_wrapped') ?? false);
        for ($i = 0; $size === null || $i < $size; ++$i) {
          if ($size === null && !$protocol->readMapHasNext()) {
            break;
          }

          $key = null;
          $val = null;
          $xfer += self::readStructHelper(
            $protocol,
            Shapes::at($tspec, 'ktype'),
            inout $key,
            Shapes::at($tspec, 'key'),
            inout $has_type_wrapper,
          );
          $xfer += self::readStructHelper(
            $protocol,
            Shapes::at($tspec, 'vtype'),
            inout $val,
            $val_spec,
            inout $has_type_wrapper,
          );

          // If the element type is enum and the enum
          // does not exist, it will return a null.
          if ($key === null || $val === null) {
            continue;
          }

          $map[ArgAssert::isArraykey($key)] = $val;
        }

        // Convert collection to the correct format.
        if (Shapes::at($tspec, 'format') === 'harray') {
          $map = $map;
        } else if (Shapes::at($tspec, 'format') === 'collection') {
          $map = new Map($map);
        } else { // format === 'array'
          $map = ($protocol->getOptions() & THRIFT_MARK_LEGACY_ARRAYS)
            ? HH\array_mark_legacy($map)
            : $map;
        }

        $object = $map;
        $xfer += $protocol->readMapEnd();
        break;
      case TType::STRUCT:
        $cls = Shapes::at($tspec, 'class');
        $struct = new $cls();
        $xfer += PHPism_FIXME::castForArithmetic($struct->read($protocol));
        $object = $struct;
        break;
      default:
        $xfer += $protocol->skip($field_type);
    }
    if ($object is nonnull) {
      $adapter = Shapes::idx($tspec, 'adapter');
      if ($adapter is nonnull) {
        $object = $adapter::fromThrift(HH\FIXME\UNSAFE_CAST<
          nonnull,
          HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
        >(
          $object,
          'FIXME[4110] This is safe as long as $adapter::TThriftType matches the thrift type of the field',
        ));
      }
    }
    $has_type_wrapper =
      $has_type_wrapper || (Shapes::idx($tspec, 'is_type_wrapped') ?? false);
    return $xfer;
  }

  public static function writeStruct(
    TProtocol $protocol,
    IThriftStruct $object,
  )[$protocol::CReadWriteDefault, write_props]: int {
    $xfer = $protocol->writeStructBegin($object->getName());
    foreach ($object::SPEC as $field_id => $fspec) {
      $field_name = $fspec['var'];
      $is_wrapped = Shapes::idx($fspec, 'is_wrapped', false);
      $field_type = $fspec['type'];
      if ($is_wrapped && !Str\is_empty($field_name)) {
        $getter_method = "get_".$field_name;
        /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
        $field_value = $object->$getter_method();
        if ($field_value is IThriftWrapper<_>) {
          $field_value = HH\FIXME\UNSAFE_CAST<
            mixed,
            HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
          >($field_value)->getValue_DO_NOT_USE_THRIFT_INTERNAL();
        } else {
          $field_value = null;
        }
      } else {
        /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
        $field_value = $object->$field_name;
      }

      $field_value = self::unwrapApplyAdapter($field_value, $fspec);
      if (self::isFieldEmpty($field_value, $fspec)) {
        continue;
      }

      $xfer += $protocol->writeFieldBegin($field_name, $field_type, $field_id);
      $xfer +=
        self::writeStructHelper($protocol, $field_type, $field_value, $fspec);
      $xfer += $protocol->writeFieldEnd();
    }
    $xfer += $protocol->writeFieldStop();
    $xfer += $protocol->writeStructEnd();
    return $xfer;
  }

  private static function isValueTypeDefault(
    mixed $field_value,
    TType $field_type,
  )[]: bool {
    switch ($field_type) {
      case TType::BOOL:
        return (bool)$field_value === false;
      case TType::BYTE:
      case TType::I16:
      case TType::I32:
      case TType::I64:
        return (int)$field_value === 0;
      case TType::DOUBLE:
      case TType::FLOAT:
        return (float)$field_value === 0.0;
      case TType::STRING:
      case TType::UTF8:
      case TType::UTF16:
        return Str\is_empty(PHPism_FIXME::stringCast($field_value));
      case TType::LST:
      case TType::SET:
      case TType::MAP:
        return PHP\count($field_value) === 0;
      case TType::STRUCT: {
        $struct = $field_value as IThriftStruct;

        // Not all unions are migrated to getters/setters,
        // but assume that they are.
        if ($struct is IThriftUnion<_>) {
          return $struct->getType() === 0;
        }

        // Naive implementation: worst-case quadratic runtime if all fields are
        // set, as nested fields are redundantly checked for terseness.
        // However, this is more efficient for sparse and small structs.
        // Potentially replace this with rollback serialization for large
        // structs.
        foreach ($struct::SPEC as $fspec) {
          $field_name = $fspec['var'];
          if (
            !self::isFieldEmpty(
              /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
              $struct->$field_name,
              $fspec,
            )
          ) {
            return false;
          }
        }

        return true;
      }
      case TType::STOP:
      case TType::VOID:
        throw new Exception('Encountered invalid type for field.');
    }
  }

  private static function isFieldEmpty(
    mixed $field_value,
    ThriftStructTypes::TGenericSpec $fspec,
  )[]: bool {
    // Optionals:
    // When a field is marked as optional and it's not set then
    // ignore the field. Otherwise, include default value.
    // Note: If an optional field has a default value,
    // it will still be added to the buffer.
    return $field_value is null ||
      (
        Shapes::idx($fspec, 'is_terse', false) &&
        self::isValueTypeDefault($field_value, $fspec['type'])
      );
  }

  private static function unwrapApplyAdapter(
    mixed $field_value,
    ThriftStructTypes::TGenericSpec $tspec,
  )[write_props]: mixed {
    if ($field_value is IThriftWrapper<_>) {
      $field_value = $field_value->getValue_DO_NOT_USE_THRIFT_INTERNAL();
    }

    $adapter = Shapes::idx($tspec, 'adapter');
    if ($field_value is nonnull && $adapter is nonnull) {
      $field_value = $adapter::toThrift(HH\FIXME\UNSAFE_CAST<
        mixed,
        HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
      >(
        $field_value,
        'FIXME[4110] This is safe as long as $adapter::TThriftType matches the thrift type of the field',
      ));
    }
    return $field_value;
  }

  public static function writeStructHelper(
    TProtocol $protocol,
    TType $field_type,
    mixed $object,
    ThriftStructTypes::TGenericSpec $tspec,
  )[$protocol::CReadWriteDefault, write_props]: int {
    $xfer = 0;
    // TODO (partisan): Analyze the types of `$object`, when the primitive type
    // is expected. This likely can wait till (if) we try to get rid of mixed
    // types
    switch ($field_type) {
      case TType::BOOL:
        $xfer += $protocol->writeBool((bool)$object);
        break;
      case TType::BYTE:
        $xfer += $protocol->writeByte((int)$object);
        break;
      case TType::I16:
        $xfer += $protocol->writeI16((int)$object);
        break;
      case TType::I32:
        $xfer += $protocol->writeI32((int)$object);
        break;
      case TType::I64:
        $xfer += $protocol->writeI64((int)$object);
        break;
      case TType::DOUBLE:
        $xfer += $protocol->writeDouble((float)$object);
        break;
      case TType::FLOAT:
        $xfer += $protocol->writeFloat((float)$object);
        break;
      case TType::STRING:
        $is_binary = Shapes::idx($tspec, 'is_binary') ?? false;
        $str = PHPism_FIXME::stringCast($object);
        $xfer += $is_binary
          ? $protocol->writeBinary($str)
          : $protocol->writeString($str);
        break;
      case TType::LST:
        $xfer += $protocol->writeListBegin(
          Shapes::at($tspec, 'etype'),
          PHP\count($object),
        );
        if ($object !== null) {
          foreach (
            PHPism_FIXME::coerceTraversableOrObject(
              HH\FIXME\UNSAFE_CAST<nonnull, Traversable<nothing>>(
                $object,
                'FIXME[4110] It should be a traversable in this branch',
              ),
            ) as $iter
          ) {
            $espec = Shapes::at($tspec, 'elem');
            $iter = self::unwrapApplyAdapter($iter, $espec);
            $xfer += self::writeStructHelper(
              $protocol,
              Shapes::at($tspec, 'etype'),
              $iter,
              $espec,
            );
          }
        }
        $xfer += $protocol->writeListEnd();
        break;
      case TType::SET:
        $xfer += $protocol->writeSetBegin(
          Shapes::at($tspec, 'etype'),
          PHP\count($object),
        );
        if ($object !== null) {
          foreach (
            PHPism_FIXME::coerceKeyedTraversableOrObject(
              HH\FIXME\UNSAFE_CAST<nonnull, KeyedTraversable<nothing, nothing>>(
                $object,
                'FIXME[4110] It should be a keyed traversable in this branch',
              ),
            ) as $key => $iter
          ) {
            $xfer += self::writeStructHelper(
              $protocol,
              Shapes::at($tspec, 'etype'),
              Shapes::at($tspec, 'format') === 'array' ? $key : $iter,
              Shapes::at($tspec, 'elem'),
            );
          }
        }
        $xfer += $protocol->writeSetEnd();
        break;
      case TType::MAP:
        $xfer += $protocol->writeMapBegin(
          Shapes::at($tspec, 'ktype'),
          Shapes::at($tspec, 'vtype'),
          PHP\count($object),
        );
        if ($object !== null) {
          foreach (
            PHPism_FIXME::coerceKeyedTraversableOrObject(
              HH\FIXME\UNSAFE_CAST<nonnull, KeyedTraversable<nothing, nothing>>(
                $object,
                'FIXME[4110] It should be a keyed traversable in this branch',
              ),
            ) as $kiter => $viter
          ) {
            $vspec = Shapes::at($tspec, 'val');
            $viter = self::unwrapApplyAdapter($viter, $vspec);

            $xfer += self::writeStructHelper(
              $protocol,
              Shapes::at($tspec, 'ktype'),
              $kiter,
              Shapes::at($tspec, 'key'),
            );
            $xfer += self::writeStructHelper(
              $protocol,
              Shapes::at($tspec, 'vtype'),
              $viter,
              $vspec,
            );
          }
        }
        $xfer += $protocol->writeMapEnd();
        break;
      case TType::STRUCT:
        if ($object !== null) {
          $xfer +=
            HH_FIXME::dynamicCastUnknownObject($object)->write($protocol);
        } else {
          $xfer += $protocol->writeStructBegin(Shapes::at($tspec, 'class'));
          $xfer += $protocol->writeStructEnd();
        }
        break;
      default:
        $xfer += $protocol->skip($field_type);
    }
    return (int)$xfer;
  }
}

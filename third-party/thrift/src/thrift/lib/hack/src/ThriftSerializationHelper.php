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
 */

/**
 * Generic Protocol Reader and Writer
 */
class ThriftSerializationHelper {
  public static function readStruct(
    TProtocol $protocol,
    $object,
  ): int {
    $field_name = '';
    $field_type = 0;
    $field_id = 0;

    $tspec = $object::$_TSPEC;
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
        if (!isset($tspec[$field_id]) ||
            $field_type !== $tspec[$field_id]['type']) {
          $xfer += $protocol->skip($field_type);
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
        if (!array_key_exists($field_name, $object::$_TFIELDMAP)) {
          $xfer += $protocol->skip($field_type);
          $xfer += $protocol->readFieldEnd();
          continue;
        }

        // This uses the TFIELDMAP to find the field id.
        $field_id = $object::$_TFIELDMAP[$field_name];
      }

      $xfer += self::readStructHelper(
        $protocol,
        $tspec[$field_id]['type'],
        &$object->$field_name,
        $tspec[$field_id]
      );

      $xfer += $protocol->readFieldEnd();
    }
    $xfer += $protocol->readStructEnd();
    return $xfer;
  }

  public static function readUnion(
    TProtocol $protocol,
    $object,
    &$union_enum,
  ): int {
    $field_name = '';
    $field_type = 0;
    $field_id = 0;

    $tspec = $object::$_TSPEC;
    $union_enum_name = get_class($object) . "Enum";
    $union_enum = $union_enum_name::_EMPTY_;
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
        if (!isset($tspec[$field_id]) ||
            $field_type !== $tspec[$field_id]['type']) {
          $xfer += $protocol->skip($field_type);
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
        // Since SimpleJSON doesn't have a field id, we use reflection
        // to inspect the object for the element.
        if (!array_key_exists($field_name, $object::$_TFIELDMAP)) {
          $xfer += $protocol->skip($field_type);
          $xfer += $protocol->readFieldEnd();
          continue;
        }

        // This uses the TFIELDMAP to find the field id.
        $field_id = $object::$_TFIELDMAP[$field_name];
      }

      $field_name_tmp = $object->$field_name;
      $xfer += self::readStructHelper(
        $protocol,
        $tspec[$field_id]['type'],
        &$field_name_tmp,
        $tspec[$field_id]
      );
      $object->$field_name = $field_name_tmp;
      $union_enum = $union_enum_name::coerce($object::$_TFIELDMAP[$field_name]);
      $xfer += $protocol->readFieldEnd();
    }
    $xfer += $protocol->readStructEnd();
    return $xfer;
  }

  private static function readStructHelper(
    TProtocol $protocol,
    $field_type,
    &$object,
    $tspec,
  ): int {
    $xfer = 0;
    switch ($field_type) {
      case TType::BOOL:
        $xfer += $protocol->readBool(&$object);
        break;
      case TType::BYTE:
        $xfer += $protocol->readByte(&$object);
        break;
      case TType::I16:
        $xfer += $protocol->readI16(&$object);
        break;
      case TType::I32:
        // Enums:
        // In Hack, enums are encoded as I32s.
        // This looks into the tspec to distinguish the two of them.
        // Optimization opportunity: Add a TType of enum and encode that to
        // the tspec to avoid this if statement.
        if (isset($tspec['enum'])) {
          $val = null;
          $xfer += $protocol->readI32(&$val);
          $enum_class = $tspec['enum'];
          $object = $enum_class::coerce($val);
        } else {
          $xfer += $protocol->readI32(&$object);
        }
        break;
      case TType::I64:
        $xfer += $protocol->readI64(&$object);
        break;
      case TType::DOUBLE:
        $xfer += $protocol->readDouble(&$object);
        break;
      case TType::FLOAT:
        $xfer += $protocol->readFloat(&$object);
        break;
      case TType::STRING:
        $xfer += $protocol->readString(&$object);
        break;
      case TType::LST:
        $size = 0;
        $element_type = 0;
        $xfer += $protocol->readListBegin(&$element_type, &$size);

        // Use the correct collection.
        $list = null;
        if ($tspec['format'] === 'harray') {
          $list = vec[];
        } else if ($tspec['format'] === 'collection') {
          $list = Vector {};
        } else { // format === 'array'
          $list = varray[];
        }

        for ($i = 0; $size === null || $i < $size; ++$i) {
          if ($size === null && !$protocol->readListHasNext()) {
            break;
          }

          $list_element = null;
          $xfer += self::readStructHelper(
            $protocol,
            $tspec['etype'],
            &$list_element,
            $tspec['elem']
          );

          // If the element type is enum and the enum
          // does not exist, it will return a null.
          if ($list_element === null) {
            continue;
          }

          /* HH_IGNORE_ERROR[4006] type can be Vector, vec, or array */
          $list[] = $list_element;
        }
        $object = $list;
        $xfer += $protocol->readListEnd();
        break;
      case TType::SET:
        $size = 0;
        $element_type = 0;
        $xfer += $protocol->readSetBegin(&$element_type, &$size);

        // Use the correct collection.
        if ($tspec['format'] === 'harray') {
          $set = keyset[];
        } else if ($tspec['format'] === 'collection') {
          $set = Set {};
        } else { // format === 'array'
          $set = array();
        }

        for ($i = 0; $size === null || $i < $size; ++$i) {
          if ($size === null && !$protocol->readSetHasNext()) {
            break;
          }

          $set_element = null;
          $xfer += self::readStructHelper(
            $protocol,
            $tspec['etype'],
            &$set_element,
            $tspec['elem']
          );

          // If the element type is enum and the enum
          // does not exist, it will return a null.
          if ($set_element === null) {
            continue;
          }

          // When using a set array(), we can't append in the normal way.
          // Therefore, we need to distinguish between the two types
          // before we add the element to the set.
          if ($tspec['format'] === 'array') {
            invariant(is_array($set), 'for hack');
            $set[$set_element] = true;
          } else {
            /* HH_IGNORE_ERROR[4006] type will be Set or keyset */
            $set[] = $set_element;
          }
        }
        $object = $set;
        $xfer += $protocol->readSetEnd();
        break;
      case TType::MAP:
        $size = 0;
        $key_type = 0;
        $value_type = 0;
        $xfer += $protocol->readMapBegin(
          inout $key_type,
          inout $value_type,
          inout $size,
        );

        // Use the correct collection.
        $map = null;
        if ($tspec['format'] === 'harray') {
          $map = dict[];
        } else if ($tspec['format'] === 'collection') {
          $map = Map {};
        } else { // format === 'array'
          $map = array();
        }

        for ($i = 0; $size === null || $i < $size; ++$i) {
          if ($size === null && !$protocol->readMapHasNext()) {
            break;
          }

          $key = null;
          $val = null;
          $xfer += self::readStructHelper(
            $protocol,
            $tspec['ktype'],
            &$key,
            $tspec['key']
          );
          $xfer += self::readStructHelper(
            $protocol,
            $tspec['vtype'],
            &$val,
            $tspec['val']
          );

          // If the element type is enum and the enum
          // does not exist, it will return a null.
          if ($key === null || $val === null) {
            continue;
          }

          $map[$key] = $val;
        }
        $object = $map;
        $xfer += $protocol->readMapEnd();
        break;
      case TType::STRUCT:
        $object = new $tspec['class']();
        $xfer += $object->read($protocol);
        break;
      default:
        $xfer += $protocol->skip($field_type);
    }
    return $xfer;
  }

  public static function writeStruct(
    TProtocol $protocol,
    $object,
  ): int {
    $xfer = $protocol->writeStructBegin($object->getName());
    foreach ($object::$_TSPEC as $field_id => $field) {
      $field_name = $field['var'];

      // Optionals:
      // When a field is marked as optional and it's not set then
      // ignore the field. Otherwise, include default value.
      // Note: If an optional field has a default value,
      // it will still be added to the buffer.
      if (!isset($object->$field_name)) {
        continue;
      }

      $field_type = $field['type'];
      $xfer += $protocol->writeFieldBegin($field_name, $field_type, $field_id);
      $xfer += self::writeStructHelper(
        $protocol,
        $field_type,
        $object->$field_name,
        $field,
      );
      $xfer += $protocol->writeFieldEnd();
    }
    $xfer += $protocol->writeFieldStop();
    $xfer += $protocol->writeStructEnd();
    return $xfer;
  }

  private static function writeStructHelper(
    TProtocol $protocol,
    $field_type,
    $object,
    $tspec,
  ): int {
    $xfer = 0;
    switch ($field_type) {
      case TType::BOOL:
        $xfer += $protocol->writeBool($object);
        break;
      case TType::BYTE:
        $xfer += $protocol->writeByte($object);
        break;
      case TType::I16:
        $xfer += $protocol->writeI16($object);
        break;
      case TType::I32:
        $xfer += $protocol->writeI32($object);
        break;
      case TType::I64:
        $xfer += $protocol->writeI64($object);
        break;
      case TType::DOUBLE:
        $xfer += $protocol->writeDouble($object);
        break;
      case TType::FLOAT:
        $xfer += $protocol->writeFloat($object);
        break;
      case TType::STRING:
        $xfer += $protocol->writeString($object);
        break;
      case TType::LST:
        $xfer += $protocol->writeListBegin($tspec['etype'], count($object));
        if ($object !== null) {
          foreach ($object ?? vec[] as $iter) {
            $xfer += self::writeStructHelper(
              $protocol,
              $tspec['etype'],
              $iter,
              $tspec['elem'],
            );
          }
        }
        $xfer += $protocol->writeListEnd();
        break;
      case TType::SET:
        $xfer += $protocol->writeSetBegin($tspec['etype'], count($object));
        if ($object !== null) {
          foreach ($object as $kiter => $iter) {
            $xfer += self::writeStructHelper(
              $protocol,
              $tspec['etype'],
              $tspec['format'] === 'array' ? $kiter : $iter,
              $tspec['elem'],
            );
          }
        }
        $xfer += $protocol->writeSetEnd();
        break;
      case TType::MAP:
        $xfer += $protocol->writeMapBegin(
          $tspec['ktype'],
          $tspec['vtype'],
          count($object),
        );
        if ($object !== null) {
          foreach ($object ?? dict[] as $kiter => $viter) {
            $xfer += self::writeStructHelper(
              $protocol,
              $tspec['ktype'],
              $kiter,
              $tspec['key'],
            );
            $xfer += self::writeStructHelper(
              $protocol,
              $tspec['vtype'],
              $viter,
              $tspec['val'],
            );
          }
        }
        $xfer += $protocol->writeMapEnd();
        break;
      case TType::STRUCT:
        if ($object !== null) {
          $xfer += $object->write($protocol);
        } else {
          $xfer += $protocol->writeStructBegin($tspec['class']);
          $xfer += $protocol->writeStructEnd();
        }
        break;
      default:
        $xfer += $protocol->skip($field_type);
    }
    return $xfer;
  }
}

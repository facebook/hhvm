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
 * Trait for Thrift Unions to call into the Serialization Helper
 */
<<Oncalls('thrift')>> // @oss-disable
trait ThriftUnionSerializationTrait implements IThriftStruct {

  public function read(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: int {
    $num_field_count = 0;
    $field_name = '';
    $field_type = null;
    $field_id = 0;

    invariant(
      $this is IThriftUnion<_>,
      "ThriftUnionSerializationTrait is meant to be used only with unions.",
    );
    $is_strict_union = $this is IThriftStrictUnion<_>;
    if ($this is IThriftStructWithClearTerseFields) {
      $this->clearTerseFields();
    }
    $tspec = $this::SPEC;
    $object_class_name = static::class;
    $union_enum_name = ArgAssert::isEnumname($object_class_name."Enum");
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
          $xfer += $protocol->skip($field_type as nonnull);
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
        if (!PHP\array_key_exists($field_name, $this::FIELDMAP)) {
          $xfer += $protocol->skip($field_type as nonnull);
          $xfer += $protocol->readFieldEnd();
          continue;
        }

        // This uses the TFIELDMAP to find the field id.
        $field_id =
          $this::FIELDMAP[nullthrows($field_name, 'Got unexpected null')];
      }

      $field_name_tmp = null;

      $has_type_wrapper = false;
      $xfer += ThriftSerializationHelper::readStructHelper(
        $protocol,
        $tspec[$field_id]['type'],
        inout $field_name_tmp,
        $tspec[$field_id],
        inout $has_type_wrapper,
      );
      if ($field_name_tmp !== null && $field_name is nonnull) {
        $is_field_wrapped =
          Shapes::idx($tspec[$field_id], 'is_wrapped') ?? false;
        if ($has_type_wrapper || $is_field_wrapped) {
          $setter_method = "set_".$field_name;
          /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
          $this->$setter_method($field_name_tmp);
        } else {
          /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
          $this->$field_name = $field_name_tmp;
          // every thrift union has a _type field
          $this_as_dynamic = HH_FIXME::dynamicCastForMissingMember($this);
          $this_as_dynamic->_type = HH\classname_to_class($union_enum_name)
            |> $$::coerce($this::FIELDMAP[$field_name as nonnull]);
        }
      }
      $xfer += $protocol->readFieldEnd();
      $num_field_count++;
      if ($is_strict_union && $num_field_count > 1) {
        throw new TProtocolException('Union field already set');
      }
    }
    $xfer += $protocol->readStructEnd();
    if ($num_field_count > 1) {
      HH\Coeffects\fb\backdoor_from_pure__DO_NOT_USE(
        ()[defaults] ==> {
          signal_log_in_psp(
            SignalDynamicLoggerDataset::SHARED_DATASET_AVOID,
            SignalDynamicLoggerProject::THRIFT_UNION_READ,
            $object_class_name,
            (string)$num_field_count,
          );
          CategorizedOBC::typedGet(ODSCategoryID::ODS_THRIFT)
            ->bumpEntityKeySampled(
              "thrift_union",
              "thrift_union.multiple_fields.read.".$object_class_name,
              1,
              OdsAggregationType::ODS_AGGREGATION_TYPE_SUM,
              10,
            );
        },
        'Operational logging of class and field counts',
      );
    }
    return $xfer;
  }

  public function write(
    TProtocol $protocol,
  )[$protocol::CReadWriteDefault, write_props]: int {
    invariant(
      $this is IThriftUnion<_>,
      "ThriftUnionSerializationTrait is meant to be used only with unions.",
    );
    $num_field_count = 0;
    $incorrect_field_set = vec[];
    $extended_thrift_union = false;
    $set_field_name = null;
    $object_class_name = static::class;
    try {
      $union_enum_name = $object_class_name."Enum";
      // checking directly to avoid an exception since serializing
      // thrift struct is a critical path, will remove the logging
      // after some data collection.
      $extended_thrift_union = !PHP\enum_exists($union_enum_name);
      if (!$extended_thrift_union) {
        $union_enum_name = ArgAssert::isEnumname($object_class_name."Enum");
        $set_field_name = HH\classname_to_class($union_enum_name)
          |> $$::assert(
            // every thrift union has a _type field
            HH_FIXME::dynamicCastForMissingMember($this)->_type,
          );
        $set_field_name = (
          HH\classname_to_class($union_enum_name) |> $$::getNames()
        )[$set_field_name];
      }
    } catch (IExceptionWithPureGetMessage $e) {
      HH\Coeffects\fb\backdoor_from_pure__DO_NOT_USE(
        ()[defaults] ==> signal_log_in_psp(
          SignalDynamicLoggerDataset::SHARED_DATASET_AVOID,
          SignalDynamicLoggerProject::THRIFT_UNION_WRITE,
          $object_class_name.
          " - Unexpected exception while trying to fetch enum union: ".
          $e->getMessage(),
        ),
        'Operational logging: enum arg assertion exceptions',
      );
    }
    $xfer = $protocol->writeStructBegin($this->getName());
    foreach ($this::SPEC as $field_id => $field) {
      $field_name = $field['var'];

      /* HH_FIXME[2011] dynamic method is allowed on non dynamic types */
      $field_value = $this->$field_name;

      if ($field_value is IThriftWrapper<_>) {
        $field_value = HH\FIXME\UNSAFE_CAST<
          mixed,
          HH_FIXME\UNKNOWN_TYPE_FOR_CAST,
        >($field_value)->getValue_DO_NOT_USE_THRIFT_INTERNAL();
      }

      // Optionals:
      // When a field is marked as optional and it's not set then
      // ignore the field. Otherwise, include default value.
      // Note: If an optional field has a default value,
      // it will still be added to the buffer.

      /* HH_FIXME[4016] Field value may not exist*/
      if (!isset($field_value)) {
        continue;
      }

      $field_type = $field['type'];
      $xfer += $protocol->writeFieldBegin($field_name, $field_type, $field_id);
      $adapter = Shapes::idx($field, 'adapter');
      if ($field_value is nonnull && $adapter is nonnull) {
        $field_value = $adapter::toThrift($field_value);
      }
      $xfer += ThriftSerializationHelper::writeStructHelper(
        $protocol,
        $field_type,
        $field_value,
        $field,
      );
      $xfer += $protocol->writeFieldEnd();
      $num_field_count++;
      if ($set_field_name !== $field_name) {
        $incorrect_field_set[] = $field_name;
      }
    }
    $xfer += $protocol->writeFieldStop();
    $xfer += $protocol->writeStructEnd();
    if (
      $extended_thrift_union ||
      !C\is_empty($incorrect_field_set) ||
      $num_field_count > 1
    ) {
      HH\Coeffects\fb\backdoor_from_pure__DO_NOT_USE(
        ()[defaults] ==> {
          if ($num_field_count > 1) {
            signal_log_in_psp(
              SignalDynamicLoggerDataset::SHARED_DATASET_AVOID,
              SignalDynamicLoggerProject::THRIFT_UNION_WRITE,
              $object_class_name,
              (string)$num_field_count,
            );
          }

          if (!C\is_empty($incorrect_field_set)) {
            signal_log_in_psp(
              SignalDynamicLoggerDataset::SHARED_DATASET_AVOID,
              SignalDynamicLoggerProject::THRIFT_UNION_INCORRECT_FIELD_SET,
              $object_class_name,
              Str\format(
                "Expected: %s, Actual: %s",
                $set_field_name ?? "(missing field)",
                Str\join($incorrect_field_set, ','),
              ),
            );
          }

          if ($extended_thrift_union) {
            signal_log_in_psp(
              SignalDynamicLoggerDataset::SHARED_DATASET_AVOID,
              SignalDynamicLoggerProject::THRIFT_UNION_EXTENDED_THRIFT_UNION,
              $object_class_name,
            );
          }

          if ($num_field_count > 1) {
            CategorizedOBC::typedGet(ODSCategoryID::ODS_THRIFT)
              ->bumpEntityKeySampled(
                "thrift_union",
                "thrift_union.multiple_fields.write.".$object_class_name,
                1,
                OdsAggregationType::ODS_AGGREGATION_TYPE_SUM,
                10,
              );
          }
        },
        'Operational logging of class and field counts',
      );
    }
    return $xfer;
  }

  private function logIncorrectFieldAccessed(
    arraykey $expected_field,
    arraykey $actual_field,
    bool $is_null = false,
  )[]: void {
    if ($expected_field !== $actual_field && !$is_null) {
      HH\Coeffects\fb\backdoor_from_pure__DO_NOT_USE(
        ()[defaults] ==> {
          if (JustKnobs::eval('thrift/hack:log_incorrect_union_field_access')) {
            signal_log_in_psp(
              SignalDynamicLoggerDataset::SHARED_DATASET_AVOID,
              SignalDynamicLoggerProject::THRIFT_UNION_INCORRECT_FIELD_ACCESS,
              nameof static,
              Str\format(
                "Expected: %s, Actual: %s",
                (string)$expected_field,
                (string)$actual_field,
              ),
            );
          }
        },
        'Operational logging of class and field counts',
      );
    }
  }
}

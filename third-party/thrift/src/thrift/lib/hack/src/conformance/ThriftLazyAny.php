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

final class ThriftLazyAny implements JsonSerializable {
  const type TAnyData = (mixed, ThriftTypeStructAdapter);

  private function __construct(
    private Either<apache_thrift_type_AnyStruct, self::TAnyData> $data,
  )[] {
  }

  const type TCustomSerializerFromId = (function(
    int,
  )[write_props]: classname<TProtocolWritePropsSerializer>);

  const type TCustomSerializerFromString = (function(
    string,
  )[write_props]: classname<TProtocolWritePropsSerializer>);

  private ?self::TCustomSerializerFromId $idToSerializer;

  private ?self::TCustomSerializerFromString $stringToSerializer;

  public function getTypeURI()[]: ?string {
    $type = $this->data->map($any ==> $any->type, $value ==> $value[1]);
    $uri = $type?->getTypeURI();
    if (
      $uri is null ||
      $uri->getType() === apache_thrift_type_standard_TypeUriEnum::_EMPTY_
    ) {
      return null;
    }
    return $uri->get_uri() ??
      ThriftTypeRegistry::getUriForHashPrefix(
        $uri->getx_typeHashPrefixSha2_256(),
      );
  }

  public static function fromAnyStruct(
    apache_thrift_type_AnyStruct $thrift_value,
  )[]: ThriftLazyAny {
    return new ThriftLazyAny(Either::left($thrift_value));
  }

  public static function fromObject<reify T>(
    T $obj,
  )[write_props]: ThriftLazyAny {
    return new ThriftLazyAny(
      Either::right(tuple($obj, ThriftTypeStructAdapter::fromHackType<T>())),
    );
  }

  public static function fromObjectWithExplicitTypeStruct(
    mixed $obj,
    ThriftTypeStructAdapter $thrift_type,
  )[]: ThriftLazyAny {
    return new ThriftLazyAny(Either::right(tuple($obj, $thrift_type)));
  }

  <<__Memoize>>
  public function get<reify T>()[leak_safe, globals]: T {
    // @lint-ignore AVOID_TYPE_ASSERT container types can't be __Enforceable
    return TypeAssert::throwsReified<T>(
      $this->getNullableUntyped() ?? self::getIntrinsicDefaultForHackType<T>(),
    );
  }

  <<__Memoize>>
  public function getNullableUntyped()[leak_safe]: mixed {
    return $this->data->map(
      $any ==> {
        $any_type = $any->type;
        if ($any_type === null) {
          if (Str\is_empty($any->data)) {
            return null;
          } else {
            throw new TApplicationException(
              "No type information available in any struct",
            );
          }
        } else {
          $protocol = $any->protocol;
          if (
            $protocol?->get_standard() ===
              apache_thrift_type_standard_StandardProtocol::Binary
          ) {
            return TBinarySerializer::deserializeData(
              $any->data,
              $any_type->toTypeSpec(),
            );
          } else {
            $serializer = $this->getSerializer($any->protocol as nonnull);
            return
              $serializer::deserializeData($any->data, $any_type->toTypeSpec());
          }
        }
      },
      $value ==> $value[0],
    );
  }

  <<__Memoize>>
  public function toAny(
    ?apache_thrift_type_standard_StandardProtocol $protocol = null,
  )[write_props]: apache_thrift_type_AnyStruct {
    return $this->data->map(
      $any ==> $any,
      $value ==> {
        $protocol =
          $protocol ?? apache_thrift_type_standard_StandardProtocol::Compact;
        $serializer = $this->getSerializerForProtocol($protocol);
        return apache_thrift_type_AnyStruct::fromShape(shape(
          'type' => $value[1],
          'protocol' => apache_thrift_type_rep_ProtocolUnion::fromShape(shape(
            "standard" => $protocol,
          )),
          'data' =>
            $serializer::serializeData($value[0], $value[1]->toTypeSpec()),
        ));
      },
    );
  }

  <<__Memoize>>
  public function toAnyUsingBinarySerializer(
  )[leak_safe]: apache_thrift_type_AnyStruct {
    return $this->data->map(
      $any ==> $any,
      $value ==> {
        return apache_thrift_type_AnyStruct::fromShape(shape(
          'type' => $value[1],
          'protocol' => apache_thrift_type_rep_ProtocolUnion::fromShape(shape(
            "standard" => apache_thrift_type_standard_StandardProtocol::Binary,
          )),
          'data' => TBinarySerializer::serializeData(
            $value[0],
            $value[1]->toTypeSpec(),
          ),
        ));
      },
    );
  }

  public function jsonSerialize(): mixed {
    return $this->getNullableUntyped();
  }

  // Internal implementation methods.
  public function setIdToSerializer(
    self::TCustomSerializerFromId $id_to_protocol,
  )[write_props]: void {
    $this->idToSerializer = $id_to_protocol;
  }

  public function setStringToSerializer(
    self::TCustomSerializerFromString $string_to_protocol,
  )[write_props]: void {
    $this->stringToSerializer = $string_to_protocol;
  }

  private function getIdToSerializer()[]: self::TCustomSerializerFromId {
    if ($this->idToSerializer === null) {
      throw new Exception(
        "Please provide a method to map protocol id to a custom protocol",
      );
    }
    return $this->idToSerializer;
  }

  private function getStringToSerializer(
  )[]: self::TCustomSerializerFromString {
    if ($this->stringToSerializer === null) {
      throw new Exception(
        "Please provide a method to map string to a custom protocol",
      );
    }
    return $this->stringToSerializer;
  }

  private function getSerializer(
    apache_thrift_type_rep_ProtocolUnion $protocol_union,
  )[write_props]: classname<TProtocolWritePropsSerializer> {
    switch ($protocol_union->getType()) {
      case apache_thrift_type_rep_ProtocolUnionEnum::standard:
        return
          $this->getSerializerForProtocol($protocol_union->getx_standard());
      case apache_thrift_type_rep_ProtocolUnionEnum::custom:
        return ($this->getStringToSerializer())($protocol_union->getx_custom());
      case apache_thrift_type_rep_ProtocolUnionEnum::id:
        return ($this->getIdToSerializer())($protocol_union->getx_id());
      case apache_thrift_type_rep_ProtocolUnionEnum::compressed:
        invariant_violation("ProtocolUnion.compressed is not supported yet.");
      case apache_thrift_type_rep_ProtocolUnionEnum::_EMPTY_:
        invariant_violation("No protocol provided for Lazy serialization");
    }
  }

  private function getSerializerForProtocol(
    apache_thrift_type_standard_StandardProtocol $protocol,
  )[write_props]: classname<TProtocolWritePropsSerializer> {
    switch ($protocol) {
      case apache_thrift_type_standard_StandardProtocol::Compact:
        return TCompactSerializer::class;
      case apache_thrift_type_standard_StandardProtocol::Binary:
        throw new Exception(
          "Incorrect use of APIs, use Binary protocol specific APIs",
        );
      case apache_thrift_type_standard_StandardProtocol::SimpleJson:
        return JSONThriftSerializer::class;
      case apache_thrift_type_standard_StandardProtocol::Json:
      case apache_thrift_type_standard_StandardProtocol::Custom:
        invariant_violation(
          "Unsupported Lazy serialization protocol: %d",
          $protocol,
        );
    }
  }

  public function getType()[write_props]: ThriftTypeStructAdapter {
    return $this->toAny()->type as nonnull;
  }

  private static function getIntrinsicDefaultForHackType<reify T>()[]: mixed {
    $ts = HH\ReifiedGenerics\get_type_structure<T>();
    switch ($ts['kind']) {
      case TypeStructureKind::OF_INT:
        return 0;
      case TypeStructureKind::OF_BOOL:
        return false;
      case TypeStructureKind::OF_FLOAT:
        return 0.0;
      case TypeStructureKind::OF_CLASS_PTR:
      case TypeStructureKind::OF_STRING:
        return "";
      case TypeStructureKind::OF_CLASS:
        $thrift_cls = Classnames::assert(
          Shapes::at($ts, 'classname'),
          IThriftStruct::class,
        );
        return HH\classname_to_class($thrift_cls) |> $$::withDefaultValues();
      case TypeStructureKind::OF_ENUM:
        return 0;
      case TypeStructureKind::OF_DICT:
        return dict[];
      case TypeStructureKind::OF_VEC:
        return vec[];
      case TypeStructureKind::OF_KEYSET:
        return keyset[];
      default:
        throw new TApplicationException(
          "Could not find intrinsic default for Hack type.",
        );
    }
  }
}

abstract class ThriftLazyAnyAdapterBase
  implements IThriftAdapter, IAmClownilyExposingAllPropertiesViaJsonSerialize {

  const type TThriftType = apache_thrift_type_AnyStruct;
  const type THackType = ThriftLazyAny;

  public static function fromThrift(
    apache_thrift_type_AnyStruct $thrift_value,
  )[]: ThriftLazyAny {
    return ThriftLazyAny::fromAnyStruct($thrift_value);
  }
}

final class ThriftLazyAnyAdapter extends ThriftLazyAnyAdapterBase {
  <<__Override>>
  public static function toThrift(
    ThriftLazyAny $hack_value,
  )[write_props]: apache_thrift_type_AnyStruct {
    return $hack_value->toAny();
  }
}

final class ThriftLazyAnySimpleJsonAdapter extends ThriftLazyAnyAdapterBase {
  <<__Override>>
  public static function toThrift(
    ThriftLazyAny $hack_value,
  )[write_props]: apache_thrift_type_AnyStruct {
    return $hack_value->toAny(
      apache_thrift_type_standard_StandardProtocol::SimpleJson,
    );
  }
}

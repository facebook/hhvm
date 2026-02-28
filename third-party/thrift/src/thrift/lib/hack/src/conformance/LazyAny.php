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

final class LazyAny implements JsonSerializable {
  const type TData = Either<apache_thrift_Any, IThriftStruct>;

  public static function fromStruct(IThriftStruct $struct)[]: LazyAny {
    return new self(Either::right($struct));
  }

  public static function fromAny(apache_thrift_Any $thrift_value)[]: LazyAny {
    return new self(Either::left($thrift_value));
  }

  <<__Memoize>>
  public function get<T as IThriftStruct>(
    class<T> $cls,
    ?apache_thrift_StandardProtocol $protocol = null,
  )[write_props]: T {
    return $this->data->map(
      $serialized ==> {
        if (self::isEmptyAny($serialized)) {
          return new $cls();
        } else {
          self::validateAny($serialized, $cls);
          $serializer = self::getSerializer($protocol ?? $serialized->protocol);
          return $serializer::deserialize($serialized->data, new $cls());
        }
      },
      $deserialized ==> ArgAssert::isInstance($deserialized, $cls),
    );
  }

  public function getReified<reify T as IThriftStruct>()[write_props]: T {
    return $this->get(HH\ReifiedGenerics\get_class_from_type<T>());
  }

  <<__Memoize>>
  public function getUsingBinarySerializer<T as IThriftStruct>(
    classname<T> $cls,
  )[leak_safe]: T {
    return $this->data->map(
      $serialized ==> {
        if (self::isEmptyAny($serialized)) {
          return HH\classname_to_class($cls) |> new $$();
        } else {
          self::validateAny($serialized, $cls);
          return TBinarySerializer::deserialize(
            $serialized->data,
            HH\classname_to_class($cls) |> new $$(),
          );
        }
      },
      $deserialized ==> ArgAssert::isInstance($deserialized, $cls),
    );
  }

  public function getUntyped(
    ?apache_thrift_StandardProtocol $protocol = null,
  )[write_props]: ?IThriftStruct {
    return $this->data->map(
      $serialized ==> {
        if (self::isEmptyAny($serialized)) {
          // Do not change internal state to allow for future calls to get(T::class).
          return null;
        }

        $type = $serialized->type;
        $hash_prefix = $serialized->typeHashPrefixSha2_256;

        $class = null;

        if ($type is nonnull) {
          $class = ThriftTypeRegistry::getHackTypeForUri($type) ?? $type;
        } else if ($hash_prefix is nonnull) {
          $class = ThriftTypeRegistry::getHackTypeForHashPrefix($hash_prefix);
        }

        if ($class is null) {
          invariant_violation(
            "Unknown lazy type (type=%s, hash=%s)",
            $serialized->type,
            map_nullable($serialized->typeHashPrefixSha2_256, PHP\bin2hex<>),
          );
        }

        return $this->get(
          Classes::assertAndLoad($class, IThriftStruct::class),
          $protocol,
        );
      },
      $deserialized ==> $deserialized,
    );
  }

  public function getUntypedUsingBinaryProtocol()[leak_safe]: ?IThriftStruct {
    return $this->data->map(
      $serialized ==> {
        if (self::isEmptyAny($serialized)) {
          // Do not change internal state to allow for future calls to get(T::class).
          return null;
        }

        $type = $serialized->type;
        $hash_prefix = $serialized->typeHashPrefixSha2_256;

        $class = null;

        if ($type is nonnull) {
          $class = ThriftTypeRegistry::getHackTypeForUri($type);
        } else if ($hash_prefix is nonnull) {
          $class = ThriftTypeRegistry::getHackTypeForHashPrefix($hash_prefix);
        }

        if ($class is null) {
          invariant_violation(
            "Unknown lazy type (type=%s, hash=%s)",
            $serialized->type,
            map_nullable($serialized->typeHashPrefixSha2_256, PHP\bin2hex<>),
          );
        }

        return $this->getUsingBinarySerializer(
          ArgAssert::isClassname($class, IThriftStruct::class),
        );
      },
      $deserialized ==> $deserialized,
    );
  }

  // Internal implementation methods.

  private function __construct(private self::TData $data)[] {}

  public function toAny(
    ?apache_thrift_StandardProtocol $protocol = null,
    bool $add_type = false,
  )[write_props]: apache_thrift_Any {
    return $this->data->map(
      $serialized ==> $serialized,
      $deserialized ==> {
        $serializer = self::getSerializer($protocol);
        return apache_thrift_Any::fromShape(shape(
          'type' => $add_type
            ? ThriftTypeRegistry::getCanonicalUriForHackType(
                Obj::className($deserialized),
              )
            : null,
          'typeHashPrefixSha2_256' => Str\slice(
            ThriftTypeRegistry::getHashForHackType(
              Obj::className($deserialized),
            ),
            0,
            ThriftUniversalName::DEFAULT_HASH_LENGTH,
          ),
          'protocol' => $protocol,
          'data' => $serializer::serialize($deserialized),
        ));
      },
    );
  }

  public function toAnyForBinary()[leak_safe]: apache_thrift_Any {
    return $this->data->map(
      $serialized ==> $serialized,
      $deserialized ==> {
        return apache_thrift_Any::fromShape(shape(
          'type' => ThriftTypeRegistry::getCanonicalUriForHackType(
            Obj::className($deserialized),
          ),
          'typeHashPrefixSha2_256' => Str\slice(
            ThriftTypeRegistry::getHashForHackType(
              Obj::className($deserialized),
            ),
            0,
            ThriftUniversalName::DEFAULT_HASH_LENGTH,
          ),
          'protocol' => apache_thrift_StandardProtocol::Binary,
          'data' => TBinarySerializer::serialize($deserialized),
        ));
      },
    );
  }

  // Returning a write_props serializer works for now, but would have to be
  // changed if binary protocol is added.
  private static function getSerializer(
    ?apache_thrift_StandardProtocol $protocol,
  )[]: class<TProtocolWritePropsSerializer> {
    switch ($protocol ?? apache_thrift_StandardProtocol::Compact) {
      case apache_thrift_StandardProtocol::Compact:
        return TCompactSerializer::class;
      case apache_thrift_StandardProtocol::Binary:
        throw new Exception(
          "Incorrect use of APIs, use Binary protocol specific APIs",
        );
      case apache_thrift_StandardProtocol::SimpleJson:
        return JSONThriftSerializer::class;
      default:
        invariant_violation(
          "Unsupported Lazy serialization protocol: %d",
          $protocol,
        );
    }
  }

  private static function isEmptyAny(apache_thrift_Any $any)[]: bool {
    if ($any->type is nonnull || $any->typeHashPrefixSha2_256 is nonnull) {
      return false;
    }
    invariant(
      Str\is_empty($any->data),
      "Uninitialized Any must not have any data",
    );
    return true;
  }

  private static function validateAny(
    apache_thrift_Any $any,
    classname<IThriftStruct> $cls,
  )[]: void {
    if ($any->type is nonnull) {
      invariant(
        $any->type === ThriftTypeRegistry::getxCanonicalUriForHackType($cls),
        'Incorrect lazy type access: requested %s, actual %s',
        $cls,
        $any->type,
      );
    } else if ($any->typeHashPrefixSha2_256 is nonnull) {
      $hash = $any->typeHashPrefixSha2_256;
      invariant(
        Str\length($hash) >= ThriftUniversalName::MIN_HASH_LENGTH &&
          Str\starts_with(ThriftTypeRegistry::getHashForHackType($cls), $hash),
        'Incorrect lazy type access: requested %s (%s), actual %s',
        $cls,
        PHP\bin2hex(ThriftTypeRegistry::getHashForHackType($cls)),
        PHP\bin2hex($hash),
      );
    } else {
      invariant_violation(
        'Incorrect lazy type access: missing type information',
      );
    }
  }

  public function jsonSerialize()[write_props]: ?IThriftStruct {
    return $this->getUntyped();
  }
}

abstract class LazyAnyThriftAdapterBase
  implements IThriftAdapter, IAmClownilyExposingAllPropertiesViaJsonSerialize {

  const type TThriftType = apache_thrift_Any;
  const type THackType = LazyAny;

  public static function fromThrift(
    apache_thrift_Any $thrift_value,
  )[]: LazyAny {
    return LazyAny::fromAny($thrift_value);
  }
}

final class LazyAnyThriftAdapter extends LazyAnyThriftAdapterBase {
  <<__Override>>
  public static function toThrift(
    LazyAny $hack_value,
  )[write_props]: apache_thrift_Any {
    return $hack_value->toAny();
  }
}

final class LazyAnySimpleJsonThriftAdapter extends LazyAnyThriftAdapterBase {
  <<__Override>>
  public static function toThrift(
    LazyAny $hack_value,
  )[write_props]: apache_thrift_Any {
    return $hack_value->toAny(apache_thrift_StandardProtocol::SimpleJson);
  }
}

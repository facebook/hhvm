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

abstract final class ThriftTypeRegistry {
  public static function isHackTypeRegistered(classname<mixed> $cls)[]: bool {
    return ThriftTypeInfoCache::getDataForKey($cls) is nonnull;
  }

  public static function getHackTypeForTypeUri(
    apache_thrift_type_standard_TypeUri $type_uri,
  )[]: ?classname<mixed> {
    switch ($type_uri->getType()) {
      case apache_thrift_type_standard_TypeUriEnum::typeHashPrefixSha2_256:
        return self::getHackTypeForHashPrefix(
          $type_uri->getx_typeHashPrefixSha2_256(),
        );
      case apache_thrift_type_standard_TypeUriEnum::uri:
        return self::getHackTypeForUri($type_uri->getx_uri());
      default:
        throw new TApplicationException(
          "Thrift type should have at least one of URI or URI hash prefix specified",
        );
    }
  }

  public static function getxHackTypeForTypeUri(
    apache_thrift_type_standard_TypeUri $type_uri,
  )[]: ?classname<mixed> {
    return self::getHackTypeForTypeUri($type_uri) as nonnull;
  }

  public static function getHackTypeForUri(string $uri)[]: ?classname<mixed> {
    $data =
      ThriftTypeInfoCache::getDataForIndex(ThriftTypeUriIndex::class, $uri);

    invariant(
      C\count($data) <= 1,
      'Multiple types matched hash prefix, lookup is ambiguous.',
    );
    return C\first_key($data);
  }

  public static function getCanonicalUriForHackType(
    classname<mixed> $cls,
  )[]: ?string {
    $data = ThriftTypeInfoCache::getDataForKey($cls);
    if ($data is null) {
      return null;
    }

    return $data['uri'];
  }

  public static function getxCanonicalUriForHackType(
    classname<mixed> $cls,
  )[]: string {
    return self::getCanonicalUriForHackType($cls) as nonnull;
  }

  public static function getHackTypeForHashPrefix(
    string $hash_prefix,
  )[]: ?classname<mixed> {
    $data = ThriftTypeUriHashPrefixCache::getData();
    return ThriftUniversalName::getHackTypeForHashPrefix($data, $hash_prefix);
  }

  public static function getHashForHackType(classname<mixed> $cls)[]: string {
    $uri = self::getxCanonicalUriForHackType($cls);
    return ThriftUniversalName::getUriHash($uri);
  }

  public static function getUriForHashPrefix(string $hash_prefix)[]: ?string {
    $hack_type = self::getHackTypeForHashPrefix($hash_prefix);
    if ($hack_type is null) {
      return null;
    }

    return self::getCanonicalUriForHackType($hack_type);
  }

  public static function getxUriForHashPrefix(string $hash_prefix)[]: string {
    return self::getUriForHashPrefix($hash_prefix) as nonnull;
  }
}

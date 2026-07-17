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

<<Oncalls('thrift'), DeadCodeBotShouldNotRemoveEntries>>
final class ThriftTypeUriHashPrefixCache
  extends SandboxCachedPureData
  implements ISandboxCacheThatMayProduceDifferentResultsInInternAndProd {

  const type TEntry = (
    string, // URI hash
    classname<mixed>,
  );
  const type TReturn = vec<self::TEntry>;

  const keyset<SandboxCacheEnvironmentType> ENVIRONMENTS = keyset[
    SandboxCacheEnvironmentType::INTERN,
    SandboxCacheEnvironmentType::PROD,
  ];

  <<__Override>>
  protected static function getDataImpl()[SBC]: this::TReturn {
    return Vec\fb\flat_map_with_key(
      ThriftTypeUriHashInnerCache::getData(),
      ($classname, $uri_hashes) ==>
        Vec\map($uri_hashes, $uri_hash ==> tuple($uri_hash, $classname)),
    )
      |> Vec\sort_by($$, $entry ==> $entry[0], Str\compare<>);
  }
}

<<Oncalls('thrift')>>
final class ThriftTypeUriHashInnerCache
  extends SandboxCachedPureDataWithLazyKeys
  implements
    ISandboxCacheThatMayProduceDifferentResultsInInternAndProd,
    ISandboxCacheOnlyShipToTheseEnvironments {
  const type TKey = classname<mixed>;
  const type TKeyReturn = vec<string>; // URI hashes

  const int NUM_SHARDS = 50;

  const keyset<SandboxCacheEnvironmentType> ENVIRONMENTS = keyset[
    SandboxCacheEnvironmentType::INTERN,
    SandboxCacheEnvironmentType::PROD,
  ];

  // This cache is only used inside another SandboxCache, so it doesn't need
  // to be shipped. If it gets called outside of SandboxCache data fetches,
  // this needs to be removed.
  const keyset<WWWBuildVaultBuildMode> ENVIRONMENT_OVERRIDE = keyset[];

  <<__Override>>
  protected static function getKeyConfig(
  )[SBC]: SandboxCacheKeyConfigBuilder<this::TKey> {
    $facts = SandboxCachedPureDataFacts::get(shape(
      'include_abstract' => false,
      'include_test' => false,
      'include_intern' => !SandboxCacheEnvironment::isProdBuild(),
    ));

    $subtypes = $facts->getTypesOfWithAttributeFilters(ThriftTypeInfo::class);
    return SandboxCacheKeyConfigBuilder::fromKeys($subtypes);
  }

  <<__Override>>
  protected static function getDataForKeyImpl(
    this::TKey $classname,
  )[SBC]: this::TKeyReturn {
    $klass = new ReflectionClass($classname);
    $params = $klass->getAttributeClass(ThriftTypeInfo::class)?->params
      as nonnull;
    $all_uris =
      Keyset\union(keyset[$params['uri']], $params['altUris'] ?? keyset[]);

    return Vec\map($all_uris, $uri ==> ThriftUniversalName::getUriHash($uri));
  }
}

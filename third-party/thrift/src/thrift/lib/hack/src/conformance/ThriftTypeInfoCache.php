<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift'), DeadCodeBotShouldNotRemoveEntries>>
final class ThriftTypeInfoCache
  extends SandboxCachedPureDataWithLazyKeys
  implements ISandboxCacheThatMayProduceDifferentResultsInInternAndProd {

  const keyset<SandboxCacheEnvironmentType> ENVIRONMENTS = keyset[
    SandboxCacheEnvironmentType::INTERN,
    SandboxCacheEnvironmentType::PROD,
  ];

  const keyset<classname<SandboxCachedDataIndex>> INDEXES =
    keyset[nameof ThriftTypeUriIndex];

  const type TKey = classname<mixed>;
  const type TKeyReturn = shape(
    'uri' => string,
    'altUris' => keyset<string>,
  );

  <<__Override>>
  protected static function getKeyConfig(
  )[SBC]: SandboxCacheKeyConfigBuilder<this::TKey> {
    return SandboxCacheKeyConfigBuilder::fromTypesOfWithAttributeFilters(
      ThriftTypeInfo::class,
      shape(
        'include_abstract' => true,
        'include_test' => true,
        'include_intern' => !SandboxCacheEnvironment::isProdBuild(),
      ),
    );
  }

  <<__Override>>
  protected static function getDataForKeyImpl(
    this::TKey $classname,
  )[write_props]: this::TKeyReturn {
    $klass = new ReflectionClass($classname);
    $params = $klass->getAttributeClass(ThriftTypeInfo::class)?->params
      as nonnull;

    return shape(
      'uri' => $params['uri'],
      'altUris' => $params['altUris'] ?? keyset[],
    );
  }
}

<<Oncalls('thrift')>>
final class ThriftTypeUriIndex extends SandboxCachedDataIndex {
  const type TDataClass = ThriftTypeInfoCache;

  <<__Override>>
  protected static function getIndexesForKeyImpl(
    self::TDataClass::TKey $_,
    self::TDataClass::TKeyReturn $data,
  )[]: keyset<string> {
    return Keyset\union(keyset[$data['uri']], keyset($data['altUris']));
  }
}

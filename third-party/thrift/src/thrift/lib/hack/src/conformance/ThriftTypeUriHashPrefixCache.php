<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

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
    $facts = SandboxCachedPureDataFacts::get(shape(
      'include_abstract' => false,
      'include_test' => false,
      'include_intern' => !SandboxCacheEnvironment::isProdBuild(),
    ));

    $subtypes = $facts->getTypesOfWithAttributeFilters(ThriftTypeInfo::class);

    return Vec\fb\flat_map(
      $subtypes,
      $classname ==> {
        $klass = new ReflectionClass($classname);
        $params = $klass->getAttributeClass(ThriftTypeInfo::class)?->params
          as nonnull;
        $all_uris =
          Keyset\union(keyset[$params['uri']], $params['altUris'] ?? keyset[]);

        return Vec\map(
          $all_uris,
          $uri ==> tuple(ThriftUniversalName::getUriHash($uri), $classname),
        );
      },
    )
      |> Vec\sort_by($$, $entry ==> $entry[0], Str\compare<>);
  }
}

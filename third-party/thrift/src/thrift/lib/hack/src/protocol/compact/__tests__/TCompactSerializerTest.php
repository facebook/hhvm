<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('thrift')>>
final class TCompactSerializerTest extends WWWTest {

  public static function provideStructs(): vec<(IThriftStruct)> {
    return vec[
      tuple(thriftshim_Kudo::withDefaultValues()),
      tuple(new thriftshim_Kudo(true, 123)),
      tuple(new thriftshim_Kudo(false, -123)),
      tuple(new thriftshim_FooBar(Vector {1.0, 2.0, 3.0, -1.0})),
    ];
  }

  public static function providerVersions(): vec<(?int)> {
    $versions = vec[tuple(null)];
    for (
      $version = TCompactProtocolBase::VERSION_LOW;
      $version <= TCompactProtocolBase::VERSION;
      $version++
    ) {
      $versions[] = tuple($version);
    }
    return $versions;
  }

  <<
    DataProvider('provideStructs', 'providerVersions'),
    JKBoolDataProvider('thrift/hack:compact_struct'),
  >>
  public async function testSerializeDeserialize(
    IThriftStruct $struct,
    ?int $version,
  ): Awaitable<void> {
    $cls = Classes::getx($struct);
    $serialized = TCompactSerializer::serialize($struct, $version);
    $deserialized_struct =
      TCompactSerializer::deserialize($serialized, new $cls(), $version);
    $serialized_again =
      TCompactSerializer::serialize($deserialized_struct, $version);
    expect($serialized_again)->toEqual($serialized);
  }
}

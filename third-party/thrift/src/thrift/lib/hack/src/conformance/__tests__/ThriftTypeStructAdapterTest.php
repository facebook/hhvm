<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<Oncalls('smint')>>
final class ThriftTypeStructAdapterTest extends WWWTest {
  use ClassLevelTest;
  public async function testFromHackType(): Awaitable<void> {
    // Struct type

    $thrift_type = ThriftTypeStructAdapter::fromHackType<
      facebook\thrift\test\ExampleStruct,
    >();
    expect(ThriftTypeStructAdapter::toThrift($thrift_type))->toBePHPEqual(
      apache_thrift_type_rep_TypeStruct::fromShape(shape(
        'name' => apache_thrift_type_standard_TypeName::fromShape(shape(
          'structType' => apache_thrift_type_standard_TypeUri::fromShape(shape(
            'uri' => 'facebook.com/thrift/test/ExampleStruct',
          )),
        )),
      )),
    );

    // Enum type

    $thrift_type =
      ThriftTypeStructAdapter::fromHackType<facebook\thrift\test\ExampleEnum>();
    expect(ThriftTypeStructAdapter::toThrift($thrift_type))->toBePHPEqual(
      apache_thrift_type_rep_TypeStruct::fromShape(shape(
        'name' => apache_thrift_type_standard_TypeName::fromShape(shape(
          'enumType' => apache_thrift_type_standard_TypeUri::fromShape(shape(
            'uri' => 'facebook.com/thrift/test/ExampleEnum',
          )),
        )),
      )),
    );

    // Native Hack type

    $thrift_type =
      ThriftTypeStructAdapter::fromHackType<dict<string, vec<int>>>();
    expect(ThriftTypeStructAdapter::toThrift($thrift_type))->toBePHPEqual(
      apache_thrift_type_rep_TypeStruct::fromShape(shape(
        'name' => apache_thrift_type_standard_TypeName::fromShape(shape(
          'mapType' => apache_thrift_type_standard_Void::Unused,
        )),
        'params' => vec[
          apache_thrift_type_rep_TypeStruct::fromShape(shape(
            'name' => apache_thrift_type_standard_TypeName::fromShape(shape(
              'stringType' => apache_thrift_type_standard_Void::Unused,
            )),
          )),
          apache_thrift_type_rep_TypeStruct::fromShape(shape(
            'name' => apache_thrift_type_standard_TypeName::fromShape(shape(
              'listType' => apache_thrift_type_standard_Void::Unused,
            )),
            'params' => vec[
              apache_thrift_type_rep_TypeStruct::fromShape(shape(
                'name' => apache_thrift_type_standard_TypeName::fromShape(shape(
                  'i64Type' => apache_thrift_type_standard_Void::Unused,
                )),
              )),
            ],
          )),
        ],
      )),
    );
  }
}

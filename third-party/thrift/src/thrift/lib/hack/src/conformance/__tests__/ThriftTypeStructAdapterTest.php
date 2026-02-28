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

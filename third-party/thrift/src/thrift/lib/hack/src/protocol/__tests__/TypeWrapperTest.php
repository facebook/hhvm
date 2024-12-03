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

<<Oncalls('thrift')>>
final class TypeWrapperTest
  extends WWWTest
  implements IAllowUnrecognizedPublicMethodsInTestClass_MIGRATION_ONLY_FIXME {
  public static async function genNestedStruct(
  ): Awaitable<WrapperTest\MyNestedStruct> {
    return await WrapperTest\MyNestedStruct::genFromShape(shape(
      'wrapped_field' => 10,
      'annotated_field' => 100,
      'adapted_type' => '1',
      'adapted__and_wrapped_type' => '10',
      'optional_adapted_and_wrapped_type' => '100',
      'double_wrapped_struct' =>
        WrapperTest\thrift_adapted_types\StructWithWrapper::fromShape(
          shape('int_field' => 20),
        ),
    ));
  }
  public static async function genTestCases(
  ): Awaitable<dict<string, vec<WrapperTest\TestMyStruct>>> {
    $nested_struct = await self::genNestedStruct();

    return dict[
      'default_values' => vec[WrapperTest\TestMyStruct::withDefaultValues()],
      'basic' => vec[
        WrapperTest\TestMyStruct::fromShape(
          shape(
            'nested_struct' => $nested_struct,
          ),
        ),
      ],
    ];
  }

  <<DataProvider('genTestCases')>>
  public function testCompact(WrapperTest\TestMyStruct $struct): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $serialized =
        TCompactSerializer::serialize($struct, null, $disable_hphp_extension);
      $deserialized = TCompactSerializer::deserialize(
        $serialized,
        WrapperTest\TestMyStruct::withDefaultValues(),
        null,
        $disable_hphp_extension,
      );
      expect($deserialized->nested_struct)->toBePHPEqual(
        $struct->nested_struct,
      );
    }
  }

  <<DataProvider('genTestCases')>>
  public function testBinary(WrapperTest\TestMyStruct $struct): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $serialized =
        TBinarySerializer::serialize($struct, $disable_hphp_extension);
      $deserialized = TBinarySerializer::deserialize(
        $serialized,
        WrapperTest\TestMyStruct::withDefaultValues(),
        $disable_hphp_extension,
      );

      expect($deserialized->nested_struct)->toBePHPEqual(
        $struct->nested_struct,
      );
    }
  }

  <<DataProvider('genTestCases')>>
  public function testJSON(WrapperTest\TestMyStruct $struct): void {
    $serialized = JSONThriftSerializer::serialize($struct);
    $deserialized = JSONThriftSerializer::deserialize(
      $serialized,
      WrapperTest\TestMyStruct::withDefaultValues(),
    );

    expect($deserialized->nested_struct)->toBePHPEqual($struct->nested_struct);
  }

  public async function testService(): Awaitable<void> {
    list($client, $_server) = LocalThriftConnection::setup<
      WrapperTest\Service1Client,
      WrapperTestServer,
    >();
    $nested_struct = await self::genNestedStruct();
    $struct = WrapperTest\TestMyStruct::fromShape(
      shape(
        'nested_struct' => $nested_struct,
      ),
    );
    expect(await $client->func1('1', $struct))->toBePHPEqual($struct);
  }
}

<<Oncalls('thrift')>>
final class WrapperTestServiceHandler implements WrapperTest\Service1AsyncIf {
  public async function func(
    string $arg1,
    ?WrapperTest\TestMyStruct $arg2,
  ): Awaitable<WrapperTest\TestMyStruct> {
    throw new Exception("unimplemented method");
  }

  public async function func1(
    string $arg1,
    ?WrapperTest\TestMyStruct $arg2,
  ): Awaitable<WrapperTest\TestMyStruct> {
    expect($arg1)->toEqual('1');

    expect($arg2?->nested_struct)->toBePHPEqual(
      await TypeWrapperTest::genNestedStruct(),
    );
    return $arg2 as nonnull;
  }

  public async function func2(
    ?WrapperTest\StructWithWrapper $arg1,
    ?WrapperTest\i64WithWrapper $arg2,
  ): Awaitable<WrapperTest\i64WithWrapper> {
    $struct = await $arg1?->genUnwrap();
    expect($struct?->int_field)->toEqual(1);
    $int = await $arg2?->genUnwrap();
    expect($int)->toEqual(10);
    return await MyTypeIntWrapper::genFromThrift<int>(3);
  }
}

<<Oncalls('thrift')>>
final class WrapperTestServer extends METAThriftServer {
  const type TProcessor = WrapperTest\Service1AsyncProcessor;

  <<__Override>>
  protected function getProcessor(): WrapperTest\Service1AsyncProcessor {
    return
      new WrapperTest\Service1AsyncProcessor(new WrapperTestServiceHandler());
  }

  <<__Override>>
  public static function getService(): string {
    return 'wrapper_test_service';
  }
}

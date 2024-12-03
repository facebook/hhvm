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
final class FieldWrapperTest extends WWWTest {
  public static async function genTestCases(
  ): Awaitable<dict<string, vec<FieldWrapperTest\MyStruct>>> {
    return dict[
      'default_values' => vec[FieldWrapperTest\MyStruct::withDefaultValues()],
      'basic' => vec[await FieldWrapperTest\MyStruct::genFromShape(
        shape(
          'wrapped_field' => 1,
          'annotated_field' => 3,
          'adapted_type' => '4',
        ),
      )],
    ];
  }

  <<DataProvider('genTestCases')>>
  public function testCompact(FieldWrapperTest\MyStruct $struct): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $serialized =
        TCompactSerializer::serialize($struct, null, $disable_hphp_extension);
      $deserialized = TCompactSerializer::deserialize(
        $serialized,
        FieldWrapperTest\MyStruct::withDefaultValues(),
        null,
        $disable_hphp_extension,
      );
      expect(MyFieldWrapper::verifyWithStruct(
        $deserialized->get_wrapped_field(),
        $deserialized,
        $struct->get_wrapped_field(),
        $struct,
      ))->toBeTrue();
      expect(MyFieldWrapper::verifyWithStruct(
        $deserialized->get_annotated_field(),
        $deserialized,
        $struct->get_annotated_field(),
        $struct,
      ))->toBeTrue();
    }
  }

  <<DataProvider('genTestCases')>>
  public function testBinary(FieldWrapperTest\MyStruct $struct): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $serialized =
        TBinarySerializer::serialize($struct, $disable_hphp_extension);
      $deserialized = TBinarySerializer::deserialize(
        $serialized,
        FieldWrapperTest\MyStruct::withDefaultValues(),
        $disable_hphp_extension,
      );

      expect(MyFieldWrapper::verifyWithStruct(
        $deserialized->get_wrapped_field(),
        $deserialized,
        $struct->get_wrapped_field(),
        $struct,
      ))->toBeTrue();
      expect(MyFieldWrapper::verifyWithStruct(
        $deserialized->get_annotated_field(),
        $deserialized,
        $struct->get_annotated_field(),
        $struct,
      ))->toBeTrue();
    }
  }

  <<DataProvider('genTestCases')>>
  public function testJSON(FieldWrapperTest\MyStruct $struct): void {
    $serialized = JSONThriftSerializer::serialize($struct);
    $deserialized = JSONThriftSerializer::deserialize(
      $serialized,
      FieldWrapperTest\MyStruct::withDefaultValues(),
    );
    expect(MyFieldWrapper::verifyWithStruct(
      $deserialized->get_wrapped_field(),
      $deserialized,
      $struct->get_wrapped_field(),
      $struct,
    ))->toBeTrue();
    expect(MyFieldWrapper::verifyWithStruct(
      $deserialized->get_annotated_field(),
      $deserialized,
      $struct->get_annotated_field(),
      $struct,
    ))->toBeTrue();
  }

  public async function testAdapterWrapperAPI(): Awaitable<void> {
    $obj1 = FieldWrapperTest\MyStruct::withDefaultValues();
    $obj2 = FieldWrapperTest\MyStruct::withDefaultValues();
    $wrapped_field1 = await MyFieldWrapper::genFromThrift<
      int,
      FieldWrapperTest\MyStruct,
    >(1, 1, $obj1);
    $wrapped_field2 = await MyFieldWrapper::genFromThrift<
      int,
      FieldWrapperTest\MyStruct,
    >(1, 1, $obj2);
    expect(MyFieldWrapper::verifySame($wrapped_field1, $wrapped_field2))
      ->toBeFalse();

    expect(MyFieldWrapper::verifySame($wrapped_field1, $wrapped_field1))
      ->toBeTrue();

    $wrapped_field2 = await MyFieldWrapper::genFromThrift<
      int,
      FieldWrapperTest\MyStruct,
    >(1, 1, $obj1);
    expect(MyFieldWrapper::verifySame($wrapped_field1, $wrapped_field2))
      ->toBeTrue();
  }

  public async function testService(): Awaitable<void> {
    list($client, $_server) = LocalThriftConnection::setup<
      FieldWrapperTest\ServiceClient,
      FieldWrapperTestServer,
    >();
    $struct = await FieldWrapperTest\MyStruct::genFromShape(
      shape(
        'wrapped_field' => 1,
        'annotated_field' => 2,
        'adapted_type' => '4',
      ),
    );
    expect(await $client->func('1', $struct))->toEqual(3);
  }
}

class MyFieldWrapper<TThriftType, TStruct as IThriftAsyncStruct>
  extends \IThriftFieldWrapper<TThriftType, TStruct> {

  <<__Override>>
  public static async function genToThrift(
    this $value,
  )[zoned_shallow]: Awaitable<TThriftType> {
    return await $value->genUnwrap();
  }

  <<__Override>>
  public static async function genFromThrift<
    <<__Explicit>> TThriftType__,
    <<__Explicit>> TThriftStruct__ as IThriftAsyncStruct,
  >(
    TThriftType__ $value,
    int $field_id,
    TThriftStruct__ $parent,
  )[zoned_shallow]: Awaitable<MyFieldWrapper<TThriftType__, TThriftStruct__>> {
    return new MyFieldWrapper($value, $field_id, $parent);
  }

  <<__Override>>
  public async function genUnwrap()[zoned_shallow]: Awaitable<TThriftType> {
    return $this->value;
  }

  <<__Override>>
  public async function genWrap(
    TThriftType $value,
  )[zoned_shallow]: Awaitable<void> {
    $this->value = $value;
  }

  public static function verifySame<
    TThriftType__,
    TThriftStruct__ as IThriftAsyncStruct,
  >(
    ?MyFieldWrapper<TThriftType__, TThriftStruct__> $obj1,
    ?MyFieldWrapper<TThriftType__, TThriftStruct__> $obj2,
  ): bool {
    if ($obj1 === $obj2) {
      return true;
    }

    return $obj1?->value === $obj2?->value &&
      $obj1?->fieldId === $obj2?->fieldId &&
      $obj1?->struct === $obj2?->struct;
  }

  public static function verifyWithStruct<
    TThriftType__,
    TThriftStruct__ as IThriftAsyncStruct,
  >(
    ?MyFieldWrapper<TThriftType__, TThriftStruct__> $obj1,
    TThriftStruct__ $struct1,
    ?MyFieldWrapper<TThriftType__, TThriftStruct__> $obj2,
    TThriftStruct__ $struct2,
  ): bool {
    if ($obj1 === $obj2) {
      return true;
    }

    if ($obj1?->struct !== $struct1 || $obj2?->struct !== $struct2) {
      return true;
    }
    return
      $obj1?->value === $obj2?->value && $obj1?->fieldId === $obj2?->fieldId;
  }
}

<<Oncalls('thrift')>>
final class FieldWrapperTestServiceHandler
  implements FieldWrapperTest\ServiceAsyncIf {
  public async function func(
    string $arg1,
    ?FieldWrapperTest\MyStruct $arg2,
  ): Awaitable<int> {
    expect($arg1)->toEqual('1');

    expect($arg2)->toBePHPEqual(await FieldWrapperTest\MyStruct::genFromShape(
      shape(
        'wrapped_field' => 1,
        'annotated_field' => 2,
        'adapted_type' => '4',
      ),
    ));
    return 3;
  }
}

<<Oncalls('thrift')>>
final class FieldWrapperTestServer extends METAThriftServer {
  const type TProcessor = FieldWrapperTest\ServiceAsyncProcessor;

  <<__Override>>
  protected function getProcessor(): FieldWrapperTest\ServiceAsyncProcessor {
    return new FieldWrapperTest\ServiceAsyncProcessor(
      new FieldWrapperTestServiceHandler(),
    );
  }

  <<__Override>>
  public static function getService(): string {
    return 'field_adapter_test_service';
  }
}

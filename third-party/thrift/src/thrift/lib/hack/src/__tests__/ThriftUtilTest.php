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
final class ThriftUtilTest extends WWWTest {

  use ClassLevelTest;

  public async function testGetUnionTypeFromShape(): Awaitable<void> {
    expect(
      ThriftUtil::getUnionTypeFromShape<
        cmux_ConstantValue,
        cmux_ConstantValueEnum,
        cmux_ConstantValue::TShape,
      >(shape()),
    )->toEqual(cmux_ConstantValueEnum::_EMPTY_);

    expect(
      ThriftUtil::getUnionTypeFromShape<
        cmux_ConstantValue,
        cmux_ConstantValueEnum,
        cmux_ConstantValue::TShape,
      >(shape('boolVal' => false)),
    )->toEqual(cmux_ConstantValueEnum::boolVal);

    expect(
      ThriftUtil::getUnionTypeFromShape<
        cmux_ConstantValue,
        cmux_ConstantValueEnum,
        cmux_ConstantValue::TShape,
      >(shape('intVal' => 0)),
    )->toEqual(cmux_ConstantValueEnum::intVal);

    expect(
      ThriftUtil::getUnionTypeFromShape<
        cmux_ConstantValue,
        cmux_ConstantValueEnum,
        cmux_ConstantValue::TShape,
      >(shape('boolVal' => null, 'intVal' => 0)),
    )->toEqual(cmux_ConstantValueEnum::intVal);

    expect(
      () ==> ThriftUtil::getUnionTypeFromShape<
        cmux_ConstantValue,
        cmux_ConstantValueEnum,
        cmux_ConstantValue::TShape,
      >(shape('boolVal' => false, 'intVal' => 0)),
    )->toThrow(InvariantViolationException::class);
  }

  public async function testToValueMap(): Awaitable<void> {
    $struct = ThriftMarkLegacyArrays_TestStruct::fromShape(shape(
      'varray' => vec[vec['foo']],
      'darray' => dict['foo' => dict['bar' => 'baz']],
      'darraySet' => dict['foo' => true],
      'subStructVarray' => vec[],
    ));
    $expected = dict[
      'varray' => vec[vec['foo']],
      'darray' => dict['foo' => dict['bar' => 'baz']],
      'darraySet' => dict['foo' => true],
      'subStructVarray' => vec[],
    ];
    expect(await ThriftUtil::genToValueMap($struct))->toEqual($expected);

    // Wrapped struct
    $struct = await FieldWrapperTest\MyStruct::genFromShape(shape(
      'wrapped_field' => 45,
      'annotated_field' => 3,
      'adapted_type' => 'hello',
      'adapted_and_wrapped_type' => "hi",
    ));
    $expected = dict[
      'wrapped_field' => 45,
      'annotated_field' => 3,
      'adapted_type' => 'hello',
      'adapted_and_wrapped_type' => "hi",
    ];
    expect(await ThriftUtil::genToValueMap($struct))->toEqual($expected);

    // Union
    $union = ThriftMarkLegacyArrays_TestUnion::fromShape(shape(
      'varray' => vec[vec['foo']],
    ));
    $expected = dict[
      'varray' => vec[vec['foo']],
      'darray' => null,
      'darraySet' => null,
      'subStructVarray' => null,
      'subUnionVarray' => null,
    ];
    expect(await ThriftUtil::genToValueMap($union))->toEqual($expected);
  }

  public async function testGenSetValue(): Awaitable<void> {
    $struct = ThriftMarkLegacyArrays_TestStruct::withDefaultValues();
    $expected = dict[
      'varray' => vec[vec['foo']],
      'darray' => dict['foo' => dict['bar' => 'baz']],
      'darraySet' => dict['foo' => true],
      'subStructVarray' => vec[],
    ];
    foreach ($struct::SPEC as $field_spec) {
      $field_name = $field_spec['var'];
      await ThriftUtil::genSetField(
        $struct,
        $field_spec,
        $expected[$field_name],
      );
      expect(await ThriftUtil::genField($struct, $field_spec))
        ->toEqual($expected[$field_name]);
    }

    // Wrapped struct
    $struct = FieldWrapperTest\MyStruct::withDefaultValues();
    $expected = dict[
      'wrapped_field' => 45,
      'annotated_field' => 3,
      'adapted_type' => 'hello',
      'adapted_and_wrapped_type' => "hi",
    ];
    foreach ($struct::SPEC as $field_spec) {
      $field_name = $field_spec['var'];
      await ThriftUtil::genSetField(
        $struct,
        $field_spec,
        $expected[$field_name],
      );
      expect(await ThriftUtil::genField($struct, $field_spec))
        ->toEqual($expected[$field_name]);
    }

    // Union
    $union = ThriftMarkLegacyArrays_TestUnion::withDefaultValues();
    $expected = dict[
      'varray' => vec[vec['foo']],
      'darray' => null,
      'darraySet' => null,
      'subStructVarray' => null,
      'subUnionVarray' => null,
    ];
    foreach ($union::SPEC as $field_spec) {
      $field_name = $field_spec['var'];
      await ThriftUtil::genSetField(
        $union,
        $field_spec,
        $expected[$field_name],
      );
      expect(await ThriftUtil::genField($union, $field_spec))
        ->toEqual($expected[$field_name]);
    }
  }
}

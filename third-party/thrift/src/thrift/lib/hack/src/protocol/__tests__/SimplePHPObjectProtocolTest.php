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

<<Oncalls('webfoundation', 'thrift')>>
final class SimplePHPObjectProtocolTest extends WWWTest {

  use ClassLevelTest;

  public function testSameAsTSimpleJSON(): void {
    $grandchild = SimpleObjectTest_TestStruct::withDefaultValues();
    $grandchild->stringval = 'grandchild';
    $grandchild->i64s = keyset[987654321, -100];
    $grandchild->int_to_bool_map = dict[15 => true, 20 => false, -1 => true];
    $child = SimpleObjectTest_TestStruct::withDefaultValues();
    $child->int32 = 15;
    $child->i64s = keyset[1234567890123];
    $child->children = vec[$grandchild, $grandchild, $grandchild];
    $child->stringval = 'child';
    $parent = SimpleObjectTest_TestStruct::withDefaultValues();
    $parent->int32 = 0;
    $parent->children = vec[$child];
    $parent->int_to_bool_map = dict[];
    $parent->stringval = 'parent';

    $json = JSONThriftSerializer::serialize($parent);

    $tsimplejson_proto = new TSimpleJSONProtocol(new TMemoryBuffer($json));
    $json_decoded = SimpleObjectTest_TestStruct::withDefaultValues();
    $json_decoded->read($tsimplejson_proto);

    $array_decoded = TSimplePHPObjectProtocol::toThriftObject(
      hh_json_decode($json),
      SimpleObjectTest_TestStruct::withDefaultValues(),
    );
    $this->validateStructsSame($json_decoded, $array_decoded);

    $array_decoded_no_json = TSimplePHPObjectProtocol::toThriftObject(
      $parent,
      SimpleObjectTest_TestStruct::withDefaultValues(),
    );
    $this->validateStructsSame($json_decoded, $array_decoded_no_json);
  }

  private function validateStructsSame(
    SimpleObjectTest_TestStruct $expect,
    SimpleObjectTest_TestStruct $actual,
  ): void {
    expect($actual->int32)->toEqual($expect->int32);
    expect($actual->stringval)->toEqual($expect->stringval);
    expect(C\count($actual->children))->toEqual(C\count($expect->children));
    for ($i = 0; $i < C\count($actual->children); $i++) {
      $this->validateStructsSame($expect->children[$i], $actual->children[$i]);
    }
    expect(C\count($actual->int_to_bool_map))->toEqual(
      C\count($expect->int_to_bool_map),
    );
    foreach ($expect->int_to_bool_map as $key => $val) {
      expect($actual->int_to_bool_map[$key])->toEqual($val);
    }
    if ($expect->i64s === null) {
      expect($actual->i64s)->toBeNull();
    } else {
      $actual_ints = nullthrows($actual->i64s, 'Got unexpected null');
      $expect_ints = nullthrows($expect->i64s, 'Got unexpected null');
      expect(C\count($actual_ints))->toEqual(C\count($expect_ints));
      foreach ($expect_ints as $i64) {
        expect(C\contains_key($actual_ints, $i64))->toBeTrue();
      }
    }
  }

  public function testRejectGibberish(): void {
    // some things depend on fb_json_encode of empty not being parseable. This
    // was probably an accident, but we preserve these semantics.
    expect(
      () ==> JSONThriftSerializer::deserialize(
        '[]',
        SimpleObjectTest_TestStruct::withDefaultValues(),
      ),
    )->toThrow(TProtocolException::class);

    expect(
      () ==> JSONThriftSerializer::deserialize(
        '{"children": ""}',
        SimpleObjectTest_TestStruct::withDefaultValues(),
      ),
    )->toThrow(TProtocolException::class);

  }
}

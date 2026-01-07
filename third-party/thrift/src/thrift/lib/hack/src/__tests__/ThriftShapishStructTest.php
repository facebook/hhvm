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

<<Oncalls('thrift_hack')>>
final class ThriftShapishStructTest extends WWWTest {
  private function roundTripSync<TStruct as IThriftShapishSyncStruct>(
    string $test_name,
    TStruct $struct,
    (function(TStruct, TStruct): bool) $eq,
  ): void {
    $shape = $struct->__toShape();
    $round_trip = $struct::__fromShape($shape);
    expect($eq($struct, $round_trip))->toBeTrue(
      "round trip failed: %s",
      $test_name,
    );
  }

  private async function genRoundTripAsync<
    TStruct as IThriftShapishAsyncStruct,
  >(
    string $test_name,
    TStruct $struct,
    (function(TStruct, TStruct): bool) $eq,
  ): Awaitable<void> {
    $shape = await $struct->__genToShape();
    $round_trip = await $struct::__genFromShape($shape);
    expect($eq($struct, $round_trip))->toBeTrue(
      "round trip failed: %s",
      $test_name,
    );
  }

  public async function testPrimitiveFields(): Awaitable<void> {
    $this->roundTripSync(
      "bool struct field",
      new apache_thrift_struct_bool(true),
      ($a, $b) ==> $a->field_1 === $b->field_1,
    );
    $this->roundTripSync(
      "byte struct field",
      new apache_thrift_struct_byte(8),
      ($a, $b) ==> $a->field_1 === $b->field_1,
    );
    $this->roundTripSync(
      "i16 struct field",
      new apache_thrift_struct_i16(42),
      ($a, $b) ==> $a->field_1 === $b->field_1,
    );
    $this->roundTripSync(
      "i32 struct field",
      new apache_thrift_struct_i32(331),
      ($a, $b) ==> $a->field_1 === $b->field_1,
    );
    $this->roundTripSync(
      "i64 struct field",
      new apache_thrift_struct_i64(514),
      ($a, $b) ==> $a->field_1 === $b->field_1,
    );
    $this->roundTripSync(
      "float struct field",
      new apache_thrift_struct_float(3.14),
      ($a, $b) ==> Math\almost_equals($a->field_1, $b->field_1),
    );
    $this->roundTripSync(
      "double struct field",
      new apache_thrift_struct_double(2.718),
      ($a, $b) ==> Math\almost_equals($a->field_1, $b->field_1),
    );
    $this->roundTripSync(
      "binary struct field",
      new apache_thrift_struct_binary("hello"),
      ($a, $b) ==> $a->field_1 === $b->field_1,
    );
    $this->roundTripSync(
      "string struct field",
      new apache_thrift_struct_string("world"),
      ($a, $b) ==> $a->field_1 === $b->field_1,
    );
  }

  public async function testContainerFields(): Awaitable<void> {
    $this->roundTripSync(
      "empty list struct field",
      new apache_thrift_struct_list_i32(vec[]),
      ($a, $b) ==> $a->field_1 === $b->field_1,
    );

    $this->roundTripSync(
      "non-empty list struct field",
      new apache_thrift_struct_list_i32(vec[1, 2, 3, 4, 5]),
      ($a, $b) ==> $a->field_1 === $b->field_1,
    );

    $this->roundTripSync(
      "empty set struct field",
      new apache_thrift_struct_set_string(keyset[]),
      ($a, $b) ==> Keyset\equal($a->field_1, $b->field_1),
    );

    $this->roundTripSync(
      "non-empty set struct field",
      new apache_thrift_struct_set_string(keyset["hello", "world"]),
      ($a, $b) ==> Keyset\equal($a->field_1, $b->field_1),
    );

    $this->roundTripSync(
      "empty map struct field",
      new apache_thrift_struct_map_i64_binary(dict[]),
      ($a, $b) ==> Dict\equal($a->field_1, $b->field_1),
    );

    $this->roundTripSync(
      "non-empty map struct field",
      new apache_thrift_struct_map_i64_binary(
        dict[1 => "thrift", 2 => "thrift 2.1"],
      ),
      ($a, $b) ==> Dict\equal($a->field_1, $b->field_1),
    );

    $this->roundTripSync(
      "nested list struct field",
      new apache_thrift_struct_list_list_string(
        vec[vec["they", "ll", "o"], vec["wo", "rld"]],
      ),
      ($a, $b) ==> $a->field_1 === $b->field_1,
    );

    $this->roundTripSync(
      "nested map struct field",
      new apache_thrift_struct_map_string_set_i32(
        dict["first" => keyset[3, 31], "second" => keyset[5, 14]],
      ),
      ($a, $b) ==> Dict\equal($a->field_1, $b->field_1),
    );

    $this->roundTripSync(
      "list<struct> struct field",
      new apache_thrift_struct_list_struct_empty(vec[
        new apache_thrift_struct_empty(),
        new apache_thrift_struct_empty(),
      ]),
      ($a, $b) ==> C\count($a->field_1) === C\count($b->field_1),
    );
  }

  public async function testOptionalFields(): Awaitable<void> {
    $eq = ($a, $b) ==> ($a->field_1 is null && $b->field_1 is null) ||
      (
        $a->field_1 is nonnull &&
        $b->field_1 is nonnull &&
        $a->field_1 == $b->field_1
      );

    $this->roundTripSync(
      "unset optional struct field",
      new apache_thrift_struct_optional_list_bool(null),
      $eq,
    );

    $this->roundTripSync(
      "non-null optional struct field",
      new apache_thrift_struct_optional_list_bool(vec[true, false]),
      $eq,
    );
  }

  public async function testCustomDefaultFields(): Awaitable<void> {
    $this->roundTripSync(
      "custom default struct field",
      new apache_thrift_struct_i32_alternative_custom_default(),
      ($a, $b) ==> $a->field_1 === $b->field_1,
    );
  }
}

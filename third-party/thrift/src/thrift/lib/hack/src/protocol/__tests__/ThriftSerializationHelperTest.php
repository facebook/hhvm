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
final class ThriftSerializationHelperTest
  extends WWWTest
  implements ISnapshotTest {

  use ClassLevelTest;

  <<__LateInit>> public SimpleStruct $simpleStruct;
  <<__LateInit>> public ContainerStruct $containerStruct;
  <<__LateInit>> public ComplexStruct $complexStruct;

  <<__Override>>
  public async function beforeEach(): Awaitable<void> {
    $this->simpleStruct = SimpleStruct::withDefaultValues();
    $this->simpleStruct->a_bool = true;
    $this->simpleStruct->a_byte = 1;
    $this->simpleStruct->a_i16 = 10;
    $this->simpleStruct->a_i32 = 1000;
    $this->simpleStruct->a_i64 = 100000;
    $this->simpleStruct->a_double = 100.001;
    $this->simpleStruct->a_string = "foo";
    $this->simpleStruct->a_binary = "bar";

    $this->containerStruct = ContainerStruct::withDefaultValues();
    $this->containerStruct->bool_list[] = false;
    $this->containerStruct->bool_list[] = true;
    $this->containerStruct->byte_list[] = 1;
    $this->containerStruct->byte_list[] = 2;
    $this->containerStruct->i16_list[] = 3;
    $this->containerStruct->i16_list[] = 4;
    $this->containerStruct->i32_list[] = 5;
    $this->containerStruct->i32_list[] = 6;
    $this->containerStruct->i64_list[] = 7;
    $this->containerStruct->i64_list[] = 8;
    $this->containerStruct->double_list[] = 1.1;
    $this->containerStruct->double_list[] = 1.2;
    $this->containerStruct->string_list[] = "foo";
    $this->containerStruct->string_list[] = "bar";
    $this->containerStruct->binary_list[] = "bar";
    $this->containerStruct->binary_list[] = "foo";

    $this->containerStruct->byte_set[] = 1;
    $this->containerStruct->byte_set[] = 2;
    $this->containerStruct->i16_set[] = 3;
    $this->containerStruct->i16_set[] = 4;
    $this->containerStruct->i32_set[] = 5;
    $this->containerStruct->i32_set[] = 6;
    $this->containerStruct->i64_set[] = 7;
    $this->containerStruct->i64_set[] = 8;
    $this->containerStruct->string_set[] = "foo";
    $this->containerStruct->string_set[] = "bar";
    $this->containerStruct->binary_set[] = "bar";
    $this->containerStruct->binary_set[] = "foo";

    $this->containerStruct->byte_bool_map[1] = true;
    $this->containerStruct->byte_bool_map[2] = false;
    $this->containerStruct->i32_i16_map[3] = 4;
    $this->containerStruct->i32_i16_map[5] = 6;
    $this->containerStruct->string_i64_map["foo"] = 7;
    $this->containerStruct->string_i64_map["bar"] = 8;

    $this->complexStruct = ComplexStruct::withDefaultValues();
    $this->complexStruct->a_simple_struct = $this->simpleStruct;
    $this->complexStruct->a_container_struct = $this->containerStruct;
    $this->complexStruct->simple_struct_list[] = $this->simpleStruct;
    $this->complexStruct->simple_struct_list[] = $this->simpleStruct;
    $this->complexStruct->container_struct_list[] = $this->containerStruct;
    $this->complexStruct->container_struct_list[] = $this->containerStruct;
    $this->complexStruct->string_simple_struct_map["foo"] = $this->simpleStruct;
    $this->complexStruct->string_simple_struct_map["bar"] = $this->simpleStruct;
    $this->complexStruct
      ->string_container_struct_map["foo"] = $this->containerStruct;
    $this->complexStruct
      ->string_container_struct_map["bar"] = $this->containerStruct;
  }

  public function testSimpleCompact(): void {
    $transport = new TMemoryBuffer();
    $protocol = new TCompactProtocolAccelerated($transport);

    // Serialize
    ThriftSerializationHelper::writeStruct($protocol, $this->simpleStruct);

    // Deserialize
    $simpleResult = SimpleStruct::withDefaultValues();
    ThriftSerializationHelper::readStruct($protocol, $simpleResult);

    expect($simpleResult)->toBePHPEqual($this->simpleStruct);
  }

  public function testSimpleBinary(): void {
    $transport = new TMemoryBuffer();
    $protocol = new TBinaryProtocolAccelerated($transport);

    // Serialize
    ThriftSerializationHelper::writeStruct($protocol, $this->simpleStruct);

    // Deserialize
    $simpleResult = SimpleStruct::withDefaultValues();
    ThriftSerializationHelper::readStruct($protocol, $simpleResult);

    expect($this->simpleStruct)->toBePHPEqual($simpleResult);
  }

  public function testSimpleSimpleJSON(): void {
    $transport = new TMemoryBuffer();
    $protocol = new TSimpleJSONProtocol($transport);

    // Serialize
    ThriftSerializationHelper::writeStruct($protocol, $this->simpleStruct);

    // Deserialize
    $simpleResult = SimpleStruct::withDefaultValues();
    ThriftSerializationHelper::readStruct($protocol, $simpleResult);

    expect($this->simpleStruct)->toBePHPEqual($simpleResult);
  }

  public function testContainerCompact(): void {
    $transport = new TMemoryBuffer();
    $protocol = new TCompactProtocolAccelerated($transport);

    // Serialize
    ThriftSerializationHelper::writeStruct($protocol, $this->containerStruct);

    // Deserialize
    $containerResult = ContainerStruct::withDefaultValues();
    ThriftSerializationHelper::readStruct($protocol, $containerResult);

    expect($this->containerStruct)->toBePHPEqual($containerResult);
  }

  public function testContainerBinary(): void {
    $transport = new TMemoryBuffer();
    $protocol = new TBinaryProtocolAccelerated($transport);

    // Serialize
    ThriftSerializationHelper::writeStruct($protocol, $this->containerStruct);

    // Deserialize
    $containerResult = ContainerStruct::withDefaultValues();
    ThriftSerializationHelper::readStruct($protocol, $containerResult);

    expect($this->containerStruct)->toBePHPEqual($containerResult);
  }

  public function testContainerJSON(): void {
    $transport = new TMemoryBuffer();
    $protocol = new TSimpleJSONProtocol($transport);

    // Serialize
    ThriftSerializationHelper::writeStruct($protocol, $this->containerStruct);

    // Deserialize
    $containerResult = ContainerStruct::withDefaultValues();
    ThriftSerializationHelper::readStruct($protocol, $containerResult);

    expect($this->containerStruct)->toBePHPEqual($containerResult);
  }

  public function testComplexCompact(): void {
    $transport = new TMemoryBuffer();
    $protocol = new TCompactProtocolAccelerated($transport);

    // Serialize
    ThriftSerializationHelper::writeStruct($protocol, $this->complexStruct);

    // Deserialize
    $complexResult = ComplexStruct::withDefaultValues();
    ThriftSerializationHelper::readStruct($protocol, $complexResult);

    expect($this->complexStruct)->toBePHPEqual($complexResult);
  }

  public function testComplexBinary(): void {
    $transport = new TMemoryBuffer();
    $protocol = new TBinaryProtocolAccelerated($transport);

    // Serialize
    ThriftSerializationHelper::writeStruct($protocol, $this->complexStruct);

    // Deserialize
    $complexResult = ComplexStruct::withDefaultValues();
    ThriftSerializationHelper::readStruct($protocol, $complexResult);

    expect($this->complexStruct)->toBePHPEqual($complexResult);
  }

  public function testComplexJSON(): void {
    $transport = new TMemoryBuffer();
    $protocol = new TSimpleJSONProtocol($transport);

    // Serialize
    ThriftSerializationHelper::writeStruct($protocol, $this->complexStruct);

    // Deserialize
    $complexResult = ComplexStruct::withDefaultValues();
    ThriftSerializationHelper::readStruct($protocol, $complexResult);

    expect($this->complexStruct)->toBePHPEqual($complexResult);
  }

  public function testTerseWrite(): void {
    $transport = new TMemoryBuffer();
    $protocol = new TCompactProtocolAccelerated($transport);
    $terse_struct = TestTerseWriteStruct::withDefaultValues();
    $terse_struct->a_i16 = 0;
    $terse_struct->a_string = '';
    $terse_struct->a_float = 0.0;
    $terse_struct->list_of_simple_structs = Vector {};
    $terse_struct->set_of_ints = Set {};
    $terse_struct->map_of_str_to_struct = Map {};

    // Serialize
    ThriftSerializationHelper::writeStruct($protocol, $terse_struct);

    // Snapshot for manual verification
    expect(JSONThriftSerializer::serialize($terse_struct))->toMatchSnapshot();

    // Deserialize
    $terse_result = TestTerseWriteStruct::withDefaultValues();
    ThriftSerializationHelper::readStruct($protocol, $terse_result);

    expect($terse_struct)->toBePHPEqual($terse_result);

    $transport->flush();
    $protocol = new TBinaryProtocolAccelerated($transport);
    // Serialize
    ThriftSerializationHelper::writeStruct($protocol, $terse_struct);

    // Deserialize
    $terse_result = TestTerseWriteStruct::withDefaultValues();
    ThriftSerializationHelper::readStruct($protocol, $terse_result);

    expect($terse_struct)->toBePHPEqual($terse_result);

  }

  public function testStructUnionTerseWrite(): void {
    $struct = TestTerseWriteStruct::fromShape(shape(
      'a_bool' => false,
      'map_of_str_to_struct' => Map {
        "foo" => ComplexStruct::fromShape(shape(
          'a_simple_struct' => SimpleStruct::fromShape(shape(
            'a_bool' => false,
          )),
          'a_simple_union' => SimpleUnion::withDefaultValues(),
        )),
      },
    ));
    $terse_struct = TestTerseWriteStruct::fromShape(shape(
      'map_of_str_to_struct' => Map {
        "foo" => ComplexStruct::withDefaultValues(),
      },
    ));

    // Serialize
    $transport = new TMemoryBuffer();
    $protocol = new TCompactProtocolAccelerated($transport);
    ThriftSerializationHelper::writeStruct($protocol, $struct);

    $terse_transport = new TMemoryBuffer();
    $terse_protocol = new TCompactProtocolAccelerated($terse_transport);
    ThriftSerializationHelper::writeStruct($terse_protocol, $terse_struct);

    expect($transport->getBuffer())->toEqual($terse_transport->getBuffer());

    // Snapshot for manual verification
    expect(JSONThriftSerializer::serialize($terse_struct))->toMatchSnapshot();
  }

  public function testCorruptedData(): void {
    $transport = new TMemoryBuffer();
    $protocol = new TBinaryProtocolAccelerated($transport);
    ThriftSerializationHelper::writeStruct(
      $protocol,
      CorruptedDataStruct1::fromShape(
        shape(
          "string_i64_map" => Map {"foo" => 1, "bar" => 10},
        ),
      ),
    );

    expect(
      () ==> ThriftSerializationHelper::readStruct(
        $protocol,
        CorruptedDataStruct2::withDefaultValues(),
      ),
    )
      ->toThrow(TTransportException::class);
  }
}

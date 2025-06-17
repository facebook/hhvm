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
final class AdapterTest extends WWWTest {
  use ClassLevelTest;
  public static function getTestCases(
  ): dict<string, (AdapterTest\Foo, AdapterTest\FooWithoutAdapters)> {
    return dict[
      'empty' => tuple(
        AdapterTest\Foo::withDefaultValues(),
        AdapterTest\FooWithoutAdapters::withDefaultValues(),
      ),
      'default_values' => tuple(
        AdapterTest\Foo::fromShape(shape(
          'intField' => '0',
          'oIntField' => '0',
          'listField' => vec[],
          'oListField' => vec[],
          'structField' => shape('str' => '0'),
          'oStructField' => shape('str' => '0'),
          'reversedListField' => vec[],
          'mapField' => dict[],
        )),
        AdapterTest\FooWithoutAdapters::fromShape(shape(
          'intField' => 0,
          'oIntField' => 0,
          'listField' => vec[],
          'oListField' => vec[],
          'structField' => AdapterTest\Bar::withDefaultValues(),
          'oStructField' => AdapterTest\Bar::withDefaultValues(),
          'reversedListField' => vec[],
          'mapField' => dict[],
        )),
      ),
      'basic' => tuple(
        AdapterTest\Foo::fromShape(shape(
          'intField' => '1',
          'oIntField' => '2',
          'listField' =>
            vec[shape('a' => '1', 'b' => 2), shape('a' => '3', 'b' => 4)],
          'oListField' => vec[
            HH\array_mark_legacy(shape('a' => '5')),
            HH\array_mark_legacy(shape()),
          ],
          'structField' => shape('str' => '42'),
          'oStructField' => shape('str' => '43'),
          'reversedListField' => vec[1, 2, 3, 4],
          'mapField' => dict[
            "1" => shape('str' => '44'),
            "2" => shape('str' => '45'),
          ],
        )),
        AdapterTest\FooWithoutAdapters::fromShape(shape(
          'intField' => 1,
          'oIntField' => 2,
          'listField' => vec['{"a":"1","b":2}', '{"a":"3","b":4}'],
          'oListField' => vec['{"a":"5"}', '[]'],
          'structField' => AdapterTest\Bar::fromShape(shape('field' => 42)),
          'oStructField' => AdapterTest\Bar::fromShape(shape('field' => 43)),
          'reversedListField' => vec[4, 3, 2, 1],
          'mapField' => dict[
            "1" => AdapterTest\Bar::fromShape(shape('field' => 44)),
            "2" => AdapterTest\Bar::fromShape(shape('field' => 45)),
          ],
        )),
      ),
    ];
  }

  <<DataProvider('getTestCases')>>
  public function testCompact(
    AdapterTest\Foo $struct,
    AdapterTest\FooWithoutAdapters $expected_no_adapter,
  ): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $serialized =
        TCompactSerializer::serialize($struct, null, $disable_hphp_extension);
      $deserialized = TCompactSerializer::deserialize(
        $serialized,
        AdapterTest\Foo::withDefaultValues(),
        null,
        $disable_hphp_extension,
      );
      expect($deserialized)->toBePHPEqual($struct);
      $no_adapter = TCompactSerializer::deserialize(
        $serialized,
        AdapterTest\FooWithoutAdapters::withDefaultValues(),
      );
      expect($no_adapter)->toBePHPEqual($expected_no_adapter);
    }
  }

  <<DataProvider('getTestCases')>>
  public function testBinary(
    AdapterTest\Foo $struct,
    AdapterTest\FooWithoutAdapters $expected_no_adapter,
  ): void {
    foreach (vec[true, false] as $disable_hphp_extension) {
      $serialized =
        TBinarySerializer::serialize($struct, $disable_hphp_extension);
      $deserialized = TBinarySerializer::deserialize(
        $serialized,
        AdapterTest\Foo::withDefaultValues(),
        $disable_hphp_extension,
      );
      expect($deserialized)->toBePHPEqual($struct);
      $no_adapter = TBinarySerializer::deserialize(
        $serialized,
        AdapterTest\FooWithoutAdapters::withDefaultValues(),
      );
      expect($no_adapter)->toBePHPEqual($expected_no_adapter);
    }
  }

  <<DataProvider('getTestCases')>>
  public function testJSON(
    AdapterTest\Foo $struct,
    AdapterTest\FooWithoutAdapters $expected_no_adapter,
  ): void {
    $serialized = JSONThriftSerializer::serialize($struct);
    $deserialized = JSONThriftSerializer::deserialize(
      $serialized,
      AdapterTest\Foo::withDefaultValues(),
    );
    expect($deserialized)->toBePHPEqual($struct);
    $no_adapter = JSONThriftSerializer::deserialize(
      $serialized,
      AdapterTest\FooWithoutAdapters::withDefaultValues(),
    );
    expect($no_adapter)->toBePHPEqual($expected_no_adapter);
  }

  public async function testService(): Awaitable<void> {
    list($client, $_server) = LocalThriftConnection::setup<
      AdapterTest\ServiceClient,
      AdapterTestServer,
    >();
    expect(await $client->func('1', AdapterTest\Foo::fromShape(shape(
      'intField' => '2',
    ))))->toEqual(vec[3, 4]);
  }
}

final class AdapterTestIntToString implements IThriftAdapter {
  const type TThriftType = int;
  const type THackType = string;
  public static function fromThrift(int $thrift_value)[]: string {
    return (string)$thrift_value;
  }

  public static function toThrift(string $hack_value)[]: int {
    return (int)$hack_value;
  }
}

final class AdapterTestReverseList implements IThriftAdapter {

  const type THackType = vec<int>;
  const type TThriftType = vec<int>;

  public static function fromThrift(vec<int> $thrift_value)[]: vec<int> {
    return Vec\reverse($thrift_value);
  }

  public static function toThrift(vec<int> $hack_value)[]: vec<int> {
    return Vec\reverse($hack_value);
  }
}

final class AdapterTestJsonToShape implements IThriftAdapter {

  const type TThriftType = string;
  const type THackType = shape(
    ?'a' => string,
    ?'b' => int,
  );

  public static function fromThrift(string $thrift_value)[]: self::THackType {
    // @lint-ignore DEPRECATED_FUNCTION35 fb_json_decode is not pure
    return PHP\json_decode($thrift_value, true) as self::THackType;
  }

  public static function toThrift(self::THackType $hack_value)[]: string {
    return
      json_encode_pure_with_fb_flags_depend_on_legacy_array_mark($hack_value);
  }
}

final class AdapterTestStructToShape implements IThriftAdapter {

  const type TThriftType = AdapterTest\Bar;
  const type THackType = shape('str' => string);

  public static function fromThrift(
    AdapterTest\Bar $thrift_value,
  )[]: self::THackType {
    return shape('str' => (string)$thrift_value->field);
  }

  public static function toThrift(
    self::THackType $hack_value,
  )[]: AdapterTest\Bar {
    return
      AdapterTest\Bar::fromShape(shape('field' => (int)$hack_value['str']));
  }
}

final class AdapterTestServiceHandler implements AdapterTest\ServiceAsyncIf {
  public async function func(
    string $arg1,
    ?AdapterTest\Foo $arg2,
  ): Awaitable<vec<int>> {
    expect($arg1)->toEqual('1');
    expect($arg2?->intField)->toEqual('2');
    return vec[3, 4];
  }
}

<<Oncalls('thrift')>>
final class AdapterTestServer extends METAThriftServer {
  const type TProcessor = AdapterTest\ServiceAsyncProcessor;

  <<__Override>>
  protected function getProcessor(): AdapterTest\ServiceAsyncProcessor {
    return
      new AdapterTest\ServiceAsyncProcessor(new AdapterTestServiceHandler());
  }

  <<__Override>>
  public static function getService(): string {
    return 'adapter_test_service';
  }
}

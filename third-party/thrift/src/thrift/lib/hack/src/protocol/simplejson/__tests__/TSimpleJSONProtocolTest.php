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
final class TSimpleJSONProtocolTest extends WWWTest {

  use ClassLevelTest;

  public static function provideStringEncodings(): dict<string, shape(
    'val' => string,
    'json' => string,
    'simple_json' => string,
  )> {
    return dict[
      'plain string' => shape(
        'val' => 'asdf',
        'json' => 'asdf',
        'simple_json' => 'asdf',
      ),
      'infinity sign' => shape(
        'val' => "\u{221e}",
        'json' => '\u221e',
        'simple_json' => '\u00e2\u0088\u009e',
      ),
      'degree sign' => shape(
        'val' => "\u{00b0}",
        'json' => '\u00b0',
        'simple_json' => '\u00c2\u00b0',
      ),
      'decimoquinta letra' => shape(
        'val' => "\u{00f1}",
        'json' => '\u00f1',
        'simple_json' => '\u00c3\u00b1',
      ),
      'binary data' => shape(
        'val' => "\x80\x81",
        'json' => '??',
        'simple_json' => '\u0080\u0081',
      ),
    ];
  }

  <<DataProvider('provideStringEncodings')>>
  public async function testEncodingDifferences(
    string $val,
    string $json,
    string $simple_json,
  ): Awaitable<void> {
    $map = dict['s' => $val];
    expect(fb_json_encode($map))->toEqual(
      "{\"s\":\"$json\"}",
      'fb_json_encode(%s) !== %s',
      $val,
      $json,
    );

    $obj = TSimpleJSONProtocolTest_StringVal::withDefaultValues();
    $obj->s = $val;
    expect(JSONThriftSerializer::serialize($obj))->toEqual(
      "{\"s\":\"$simple_json\"}",
      'JSONThriftSerializer::serialize(%s) !== %s',
      $val,
      $simple_json,
    );
  }

  public static function provideStringDecodings(): dict<string, shape(
    'val' => string,
    'json' => string,
    'simple_json' => string,
  )> {
    return dict[
      'plain string' => shape(
        'val' => 'asdf',
        'json' => 'asdf',
        'simple_json' => 'asdf',
      ),
      'infinity sign' => shape(
        'val' => "\u{221e}",
        'json' => "\u{221e}",
        'simple_json' => "\u{221e}",
      ),
      'degree sign (JSON encoding)' => shape(
        'val' => '\u00b0',
        'json' => "\u{00b0}",
        'simple_json' => "\260",
      ),
      'degree sign (TSimpleJSON encoding)' => shape(
        'val' => '\u00c2\u00b0',
        'json' => "\303\202\302\260",
        'simple_json' => "\u{00b0}",
      ),
      'decimoquinta letra (JSON encoding)' => shape(
        'val' => '\u00f1',
        'json' => "\u{00f1}",
        'simple_json' => "\361",
      ),
      'decimoquinta letra (TSimpleJSON encoding)' => shape(
        'val' => '\u00c3\u00b1',
        'json' => "\303\203\302\261",
        'simple_json' => "\u{00f1}",
      ),
      'binary data (TSimpleJSON encoding)' => shape(
        'val' => '\u0080\u0081',
        'json' => "\302\200\302\201",
        'simple_json' => "\x80\x81",
      ),
    ];
  }

  <<DataProvider('provideStringDecodings')>>
  public async function testDecodingDifferences(
    string $val,
    string $json,
    string $simple_json,
  ): Awaitable<void> {
    $serialized_val = "{\"s\":\"$val\"}";

    $json_deserialized_val = fb_json_decode($serialized_val);
    expect($json_deserialized_val)->toEqual(
      dict['s' => $json],
      'fb_json_decode(%s) !== %s, is %s',
      $val,
      $json,
      self::octalEncodedString($json_deserialized_val['s']),
    );

    $obj = JSONThriftSerializer::deserialize(
      $serialized_val,
      TSimpleJSONProtocolTest_StringVal::withDefaultValues(),
    );
    expect($obj->s)->toEqual(
      $simple_json,
      'JSONThriftSerializer::deserialize(%s, ...) !== %s, is %s',
      $val,
      $simple_json,
      self::octalEncodedString($obj->s),
    );
  }

  public static function testEncodeDecodeNums(): void {
    $obj = TSimpleJSONProtocolTest_NumVals::withDefaultValues();
    $obj->i = -42;
    $obj->f = 100.000056;
    $obj->m = dict[1 => 0, 5 => -5];

    $serialized = JSONThriftSerializer::serialize($obj);

    expect($serialized)->toEqual('{"i":-42,"f":100.000056,"m":{"1":0,"5":-5}}');

    $deserialized = JSONThriftSerializer::deserialize(
      $serialized,
      TSimpleJSONProtocolTest_NumVals::withDefaultValues(),
    );
    expect($deserialized->i)->toEqual($obj->i);
    expect($deserialized->f)->toEqual($obj->f);
    expect($deserialized->m)->toEqual($obj->m);
  }

  public static function provideSpecialFloatingPointEncodings(): dict<
    string,
    shape(
      'write_val' => float,
      'read_val' => float,
      'simple_json' => string,
    ),
  > {
    return dict[
      'positive Infinity' => shape(
        'write_val' => INF,
        'read_val' => INF,
        'simple_json' => 'Infinity',
      ),
      'negative Infinity' => shape(
        'write_val' => -INF,
        'read_val' => -INF,
        'simple_json' => '-Infinity',
      ),
      'positive NaN' => shape(
        'write_val' => Math\NAN,
        'read_val' => Math\NAN,
        'simple_json' => 'NaN',
      ),

      // This is the odd one out: NaN is unsigned in PHP/Hack,
      // so the encoded value drops the sign.
      'negative NaN' => shape(
        'write_val' => -Math\NAN,
        'read_val' => Math\NAN,
        'simple_json' => 'NaN',
      ),
    ];
  }

  <<DataProvider('provideSpecialFloatingPointEncodings')>>
  public function testSpecialFloatingPointEncodings(
    float $write_val,
    float $read_val,
    string $simple_json,
  ): void {
    $obj = TSimpleJSONProtocolTest_NumVals::fromShape(shape(
      'f' => $write_val,
    ));
    $serialized_val = JSONThriftSerializer::serialize($obj);
    expect($serialized_val)->toEqual(
      "{\"i\":0,\"f\":\"$simple_json\",\"m\":{}}",
      'JSONThriftSerializer::serialize(%f) !== %s',
      $write_val,
      $simple_json,
    );

    $obj = JSONThriftSerializer::deserialize(
      $serialized_val,
      TSimpleJSONProtocolTest_NumVals::withDefaultValues(),
    );

    expect(
      // Need a custom comparison check here for NaN values
      (Math\is_nan($obj->f) && Math\is_nan($read_val)) ||
      /* @lint-ignore FLOATING_POINT_COMPARISON */
        ($obj->f === $read_val),
    )->toBeTrue(
      'JSONThriftSerializer::deserialize(%s, ...) !== %s',
      $simple_json,
      (string)$read_val,
    );
  }

  private static function octalEncodedString(string $str): string {
    return Str\chunk($str)
      |> Vec\map($$, (string $chr) ==> '\\'.PHP\decoct(PHP\ord($chr)))
      |> Str\join($$, '');
  }

  public static function provideBase64StringEncodings(): dict<string, shape(
    'val' => string,
    'simple_json' => string,
  )> {
    return dict[
      'plain string' => shape(
        'val' => 'abc',
        'simple_json' => 'YWJj',
      ),
      'infinity sign' => shape(
        'val' => "\u{221e}",
        'simple_json' => '4oie',
      ),
      'degree sign' => shape(
        'val' => "\u{00b0}",
        'simple_json' => 'wrA=',
      ),
      'decimoquinta letra' => shape(
        'val' => "\u{00f1}",
        'simple_json' => 'w7E=',
      ),
      'binary data' => shape(
        'val' => "\x80\x81",
        'simple_json' => 'gIE=',
      ),
    ];
  }

  <<DataProvider('provideBase64StringEncodings')>>
  public function testBinaryAsBase64(string $val, string $simple_json): void {
    $obj = TSimpleJSONProtocolTest_BinaryVal::withDefaultValues();
    $obj->s = $val;
    $serialized_val = JSONThriftSerializer::serializeBinaryAsBase64($obj);
    expect($serialized_val)->toEqual(
      "{\"s\":\"$simple_json\"}",
      'JSONThriftSerializer::serializeBinaryAsBase64(%s) !== %s',
      $val,
      $simple_json,
    );

    $obj = JSONThriftSerializer::deserializeBinaryAsBase64(
      $serialized_val,
      TSimpleJSONProtocolTest_BinaryVal::withDefaultValues(),
    );
    expect($obj->s)->toEqual(
      $val,
      'JSONThriftSerializer::deserializeBinaryAsBase64(%s, ...) !== %s, is %s',
      $simple_json,
      $val,
      self::octalEncodedString($obj->s),
    );
  }

  public function testQuoteString(): void {
    // Test with a simple string
    $input = '{"s": "Hello, World!"}';
    $expected = '"{\"s\": \"Hello, World!\"}"';
    $actual = BypassVisibility::invokeStaticMethod(
      TSimpleJSONProtocol::class,
      "quoteString",
      $input,
    );
    expect($actual)->toEqual($expected);
    // Test with a string containing special characters
    $input = '{"s": "Hello\n\r\t\b\f\\\"World!"}';
    $expected =
      '"{\"s\": \"Hello\\\\n\\\\r\\\\t\\\\b\\\\f\\\\\\\\\\"World!\"}"';
    $actual = BypassVisibility::invokeStaticMethod(
      TSimpleJSONProtocol::class,
      "quoteString",
      $input,
    );
    expect($actual)->toEqual($expected);
    // Test with an empty string
    $input = '{"s": ""}';
    $expected = '"{\"s\": \"\"}"';
    $actual = BypassVisibility::invokeStaticMethod(
      TSimpleJSONProtocol::class,
      "quoteString",
      $input,
    );
    expect($actual)->toEqual($expected);
    // Test with a null string
    $input = '{"s": null}';
    $expected = '"{\"s\": null}"';
    $actual = BypassVisibility::invokeStaticMethod(
      TSimpleJSONProtocol::class,
      "quoteString",
      $input,
    );
    expect($actual)->toEqual($expected);
    // Test with a string containing Unicode characters
    $input = '{"s": "Hello, \u{00A9} World!"}';
    $expected = '"{\"s\": \"Hello, \\\\u{00A9} World!\"}"';
    $actual = BypassVisibility::invokeStaticMethod(
      TSimpleJSONProtocol::class,
      "quoteString",
      $input,
    );
    expect($actual)->toEqual($expected);
    // Test with a string containing escaped characters
    $input = '{"s": "Hello, \\\\ World!"}';
    $expected = '"{\"s\": \"Hello, \\\\\\\\ World!\"}"';
    $actual = BypassVisibility::invokeStaticMethod(
      TSimpleJSONProtocol::class,
      "quoteString",
      $input,
    );
    expect($actual)->toEqual($expected);
    // Test with a string containing a backslash
    $input = '{"s": "Hello, \ World!"}';
    $expected = '"{\"s\": \"Hello, \\\\ World!\"}"';
    $actual = BypassVisibility::invokeStaticMethod(
      TSimpleJSONProtocol::class,
      "quoteString",
      $input,
    );
    expect($actual)->toEqual($expected);
  }

}

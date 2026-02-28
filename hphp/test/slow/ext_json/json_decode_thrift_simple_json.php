<?hh

function test_cases(): dict<string, shape(
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
    'invalid format (not \u00XX)' => shape(
      'val' => '\u1234',
      'json' => "\xe1\x88\xb4",
      'simple_json' => null,
    ),
  ];
}

function str2hex(string $str): string {
  $out = '';
  for ($i = 0; $i < strlen($str); $i++) {
    $out .= '\\x'.bin2hex($str[$i]);
  }
  return $out;
}

function report_failure(
  string $test_case_name,
  string $decoding_format,
  bool $use_simple_parser,
  ?string $expected,
  ?string $result,
): void {
  echo "Decoding $test_case_name (format: $decoding_format) failed\n";
  echo "Using ".($use_simple_parser ? "simple" : "full")." parser\n";
  echo "Expected: (".str2hex($expected ?? "\x00").") $expected\n";
  echo "Observed: (".str2hex($result ?? "\x00").") $result\n";
  echo "\n";
}

<<__EntryPoint>>
function main() :mixed{
  $pass = true;
  foreach (vec[true, false] as $use_simple_parser) {
    foreach (test_cases() as $test_case_name => $test_case) {
      $encoded = $test_case['val'];

      $expected_json = $test_case['json'];
      $result_json = json_decode(
        "\"$encoded\"",
        true,
        // any values above and below SimpleParser::kMaxArrayDepth will do
        $use_simple_parser ? 512 : 16,
      );
      if ($result_json !== $expected_json) {
        $pass = false;
        report_failure(
          $test_case_name,
          'json',
          $use_simple_parser,
          $expected_json,
          $result_json,
        );
      }

      $expected_simple = $test_case['simple_json'];
      $result_simple = json_decode(
        "\"$encoded\"",
        true,
        // any values above and below SimpleParser::kMaxArrayDepth will do
        $use_simple_parser ? 512 : 16,
        JSON_FB_THRIFT_SIMPLE_JSON,
      );
      if ($result_simple !== $expected_simple) {
        $pass = false;
        report_failure(
          $test_case_name,
          'simple_json',
          $use_simple_parser,
          $expected_simple,
          $result_simple,
        );
      }
    }
  }
  if ($pass) {
    echo "OK\n";
  }
}

<?hh

<<__EntryPoint>>
function main(): void {
  $data = vec[
    'null',
    'false',
    'true',
    '"abc"',
    '"ab\"c"',
    '0',
    '0.45',
    '-0.5',
    'invalid',
  ];

  foreach ($data as $str) {
    echo "JSON: $str\n";
    $error = null;
    $result = json_decode_with_error($str, inout $error);
    var_dump($result);
    echo "Error: ", $error[0] ?? '0', "\n";
  }
}

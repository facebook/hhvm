<?hh

function test_decode() :mixed{
  $json = vec[
    // A valid json string
    '{"Organization": "PHP Documentation Team"}',
    // An invalid json string which will cause an syntax
    // error, in this case we used ' instead of " for quotation
    "{'Organization': 'PHP Documentation Team'}",
    // A valid json string
    "",
    // A valid json string
    "{}",
  ];

  foreach ($json as $string) {
    echo 'Decoding: '.$string."\n";

    $error = null;
    json_decode_with_error($string, inout $error);
    var_dump($error);

    echo "\n";
  }
}

function test_encode() :mixed{
  $json = vec[
    // A valid json string
    "hello",
    // Invalid UTF-8
     "\x9f",
  ];

  foreach ($json as $string) {
    echo 'Encoding: '.$string."\n";

    $error = null;
    json_encode_with_error($string, inout $error);
    var_dump($error);

    echo "\n";
  }
}

<<__EntryPoint>>
function main_json_last_error() :mixed{
  test_decode();
  test_encode();
}

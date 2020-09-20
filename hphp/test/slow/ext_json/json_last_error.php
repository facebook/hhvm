<?hh

function test_decode() {
  $json = varray[
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

  $error = null;
  foreach ($json as $string) {
    echo 'Decoding: '.$string."\n";

    json_decode($string);
    echo json_last_error().": ".json_last_error_msg()."\n";

    json_decode_with_error($string, inout $error);
    var_dump($error);

    echo "\n";
  }
}

function test_encode() {
  $json = varray[
    // A valid json string
    "hello",
    // Invalid UTF-8
     "\x9f",
  ];

  $error = null;
  foreach ($json as $string) {
    echo 'Encoding: '.$string."\n";

    json_encode($string);
    echo json_last_error().": ".json_last_error_msg()."\n";

    json_encode_with_error($string, inout $error);
    var_dump($error);

    echo "\n";
  }
}

<<__EntryPoint>>
function main_json_last_error() {
  test_decode();
  test_encode();
}

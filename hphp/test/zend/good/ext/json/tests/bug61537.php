<?hh

<<__EntryPoint>>
function main(): void {
  $invalid_utf8 = "\x9f";

  $error = null;
  $result = json_encode_with_error($invalid_utf8, inout $error);
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  $error = null;
  $result = json_encode_with_error(
    $invalid_utf8,
    inout $error,
    JSON_PARTIAL_OUTPUT_ON_ERROR,
  );
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  echo "\n";

  $invalid_utf8 = "an invalid sequen\xce in the middle of a string";

  $error = null;
  $result = json_encode_with_error($invalid_utf8, inout $error);
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  $error = null;
  $result = json_encode_with_error(
    $invalid_utf8,
    inout $error,
    JSON_PARTIAL_OUTPUT_ON_ERROR,
  );
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');
}

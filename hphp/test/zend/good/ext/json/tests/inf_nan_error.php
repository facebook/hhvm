<?hh

<<__EntryPoint>>
function main(): void {
  $inf = INF;

  var_dump($inf);

  $error = null;
  $result = json_encode_with_error($inf, inout $error);
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  $error = null;
  $result = json_encode_with_error(
    $inf,
    inout $error,
    JSON_PARTIAL_OUTPUT_ON_ERROR,
  );
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  echo "\n";

  $nan = NAN;

  var_dump($nan);

  $error = null;
  $result = json_encode_with_error($nan, inout $error);
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  $error = null;
  $result = json_encode_with_error(
    $nan,
    inout $error,
    JSON_PARTIAL_OUTPUT_ON_ERROR,
  );
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');
}

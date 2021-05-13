<?hh

<<__EntryPoint>>
function main(): void {
  $a = new stdclass;
  $a->prop = $a;

  var_dump($a);

  echo "\n";

  $error = null;
  $result = json_encode_with_error($a, inout $error);
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  echo "\n";

  var_dump(json_encode($a, JSON_PARTIAL_OUTPUT_ON_ERROR));
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  echo "Done\n";
}

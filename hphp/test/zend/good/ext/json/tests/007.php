<?hh

<<__EntryPoint>>
function main(): void {
  $error = null;
  $result = json_decode_with_error("[1]", inout $error);
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  $error = null;
  $result = json_decode_with_error("[[1]]", inout $error, false, 2);
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  $error = null;
  $result = json_decode_with_error("[1}", inout $error);
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  $error = null;
  $result = json_decode_with_error('["' . chr(0) . 'abcd"]', inout $error);
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');

  $error = null;
  $result = json_decode_with_error("[1", inout $error);
  var_dump($result);
  var_dump($error[0] ?? 0, $error[1] ?? 'No error');


  echo "Done\n";
}

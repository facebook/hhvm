<?hh

<<__EntryPoint>>
function main(): void {
  $error = null;
  json_decode_with_error('{"test":"test"}', inout $error);
  var_dump($error[0] ?? 0);

  $error = null;
  json_decode_with_error("", inout $error);
  var_dump($error[0] ?? 0);

  $error = null;
  json_decode_with_error("invalid json", inout $error);
  var_dump($error[0] ?? 0);

  $error = null;
  json_decode_with_error("", inout $error);
  var_dump($error[0] ?? 0);
}

<?hh

<<__EntryPoint>>
function main(): void {
  $error = null;
  json_decode_with_error("a", inout $error);
  var_dump($error);

  $error = null;
  json_encode_with_error("", inout $error);
  var_dump($error);
}

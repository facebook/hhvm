<?hh

<<__EntryPoint>>
function main(): void {
  $data = "\xB1\x31";
  $error = null;
  $data = json_encode_with_error($data, inout $error);
  var_dump($error[0]);
}

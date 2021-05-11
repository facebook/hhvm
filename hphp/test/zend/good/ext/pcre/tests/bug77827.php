<?hh

<<__EntryPoint>>
function main(): void {
  $error = null;
  $result = preg_match_with_error("/foo/i\r", 'FOO', inout $error);
  var_dump($result, $error);
}

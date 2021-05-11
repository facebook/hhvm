<?hh

<<__EntryPoint>>
function main(): void {
  $string = b"\xc3\xa9 uma string utf8 bem formada";

  $m = null;
  $error = null;
  var_dump(preg_match_with_matches_and_error(b'~.*~u', $string, inout $m, inout $error, 0, 1));
  var_dump($m);
  var_dump($error === PREG_BAD_UTF8_OFFSET_ERROR);

  var_dump(preg_match_with_matches_and_error(b'~.*~u', $string, inout $m, inout $error, 0, 2));
  var_dump($m);
  var_dump($error === null);

  echo "Done\n";
}

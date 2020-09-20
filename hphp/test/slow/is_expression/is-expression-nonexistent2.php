<?hh

function is_nonexistent(mixed $x): void {
  if ($x is BlerpityBlerp) {
    echo "unreached\n";
  }
}
<<__EntryPoint>>
function entrypoint_isexpressionnonexistent2(): void {

  set_error_handler(($no, $string) ==> {
    throw new Exception($string);
  });

  is_nonexistent(new stdClass());
}

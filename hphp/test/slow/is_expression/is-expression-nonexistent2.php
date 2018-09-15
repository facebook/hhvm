<?hh

set_error_handler(($no, $string) ==> {
  throw new Exception($string);
});

function is_nonexistent(mixed $x): void {
  if ($x is BlerpityBlerp) {
    echo "unreached\n";
  }
}

is_nonexistent(new stdClass());

<?hh

function is_nonexistent(mixed $x): void {
  if ($x is BlerpityBlerp) {
    echo "unreached\n";
  }
}

is_nonexistent(new stdClass());

<?hh

function is_T(mixed $x): void {
  if ($x is this::T) {
    echo "T\n";
  } else {
    echo "not T\n";
  }
}

is_T(new stdClass());

<?hh

function is_soft(mixed $x): void {
  if ($x is @int) {
    echo "unreached\n";
  }
}

is_function(1);

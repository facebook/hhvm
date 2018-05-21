<?hh

function is_function(mixed $x): void {
  if ($x is (function(): int)) {
    echo "unreached\n";
  }
}

is_function(() ==> 1);

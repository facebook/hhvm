<?hh

type Tfunction = (function(): int);

function is_function(mixed $x): void {
  if ($x is Tfunction) {
    echo "unreached\n";
  }
}

is_function(() ==> 1);

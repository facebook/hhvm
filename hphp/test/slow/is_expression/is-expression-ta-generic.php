<?hh

type Tgeneric<T> = (int, string, T);

function is_generic(mixed $x): void {
  if ($x is Tgeneric) {
    echo "unreached\n";
  }
}

is_generic(tuple(1, 'foo', true));

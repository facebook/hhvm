<?hh

function f<T>(mixed $x): void {
  if ($x is T) {
    echo "T\n";
  } else {
    echo "not T\n";
  }
}

f(42);

<?hh

function f(?int $x): void {
  if ($x <> null) {
    echo $x;
  }
}

f(1);

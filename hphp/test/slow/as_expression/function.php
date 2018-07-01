<?hh

function f(mixed $x): void {
  try {
    var_dump($x as (function(): int));
  } catch (TypeAssertionException $_) {
    echo "not function: ".gettype($x)."\n";
  }
}

f(() ==> 1);

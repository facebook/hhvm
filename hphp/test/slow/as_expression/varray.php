<?hh

function f(mixed $x): void {
  try {
    var_dump($x as varray);
  } catch (TypeAssertionException $_) {
    echo "not varray: ".gettype($x)."\n";
  }
}

f(varray[]);

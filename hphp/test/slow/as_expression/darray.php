<?hh

function f(mixed $x): void {
  try {
    var_dump($x as darray);
  } catch (TypeAssertionException $_) {
    echo "not darray: ".gettype($x)."\n";
  }
}


<<__EntryPoint>>
function main_darray() {
f(darray[]);
}

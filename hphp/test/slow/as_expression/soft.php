<?hh

function f(mixed $x): void {
  try {
    var_dump($x as @int);
  } catch (TypeAssertionException $_) {
    echo "not int: ".gettype($x)."\n";
  }
}


<<__EntryPoint>>
function main_soft() {
f(1);
}

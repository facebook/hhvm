<?hh

function f(mixed $x): void {
  try {
    var_dump($x as <<__Soft>> int);
  } catch (TypeAssertionException $_) {
    echo "not int: ".gettype($x)."\n";
  }
}


<<__EntryPoint>>
function main_soft() :mixed{
f(1);
}

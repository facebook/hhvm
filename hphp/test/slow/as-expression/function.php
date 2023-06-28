<?hh

function f(mixed $x): void {
  try {
    var_dump($x as (function(): int));
  } catch (TypeAssertionException $_) {
    echo "not function: ".gettype($x)."\n";
  }
}


<<__EntryPoint>>
function main_function() :mixed{
f(() ==> 1);
}

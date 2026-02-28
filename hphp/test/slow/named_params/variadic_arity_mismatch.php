<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function try_func($f) :mixed{
  try {
    $f();
    echo "Function did not throw\n";
  } catch (Exception $e) {
    printf("Caught %s: %s\n", get_class($e), $e->getMessage());
  }
}

function foo(int $x, named int $a, named int $b, int ...$args) {
}

<<__EntryPoint>>
function main() {
    try_func(() ==> foo(1, a=3));
    try_func(() ==> foo(1, a=3, b=4));
    try_func(() ==> foo(a=3, b=4, c=5));
    try_func(() ==> foo(1, 2, b=4));
    try_func(() ==> foo(1, 2, a=3, b=4));
    try_func(() ==> foo(a=3, b=4));
}

<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

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

function too_many_exact(named int $a, named int $b, int $x) {
}

function too_many_optional_pos(named int $a, named int $b, int $x = 0) {
}

function too_many_exact_optional_named(named int $a, named int $b = 0, int $x) {
}

<<__EntryPoint>>
function main() {
    try_func(() ==> foo(1, a=3));
    try_func(() ==> foo(1, a=3, b=4));
    try_func(() ==> foo(a=3, b=4, c=5));
    try_func(() ==> foo(1, 2, b=4));
    try_func(() ==> foo(1, 2, a=3, b=4));
    try_func(() ==> foo(a=3, b=4));
    try_func(() ==> too_many_exact(a=3, b=4, 123, 124));
    try_func(() ==> too_many_optional_pos(a=3, b=4, 123, 124));
    try_func(() ==> too_many_exact_optional_named(a=3, 123, 124));
}

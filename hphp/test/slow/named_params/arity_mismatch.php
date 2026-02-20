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

function foo(int $x, int $y, named int $a, named int $b) {
}

function takes_one_named(named int $x) {
}

function takes_one_named_one_named_optional(named int $x, named int $y = 1) {
}

function takes_no_named(int $x) {
}

<<__EntryPoint>>
function main() {
    try_func(() ==> foo(1, 2, a=3));
    try_func(() ==> foo(1, 2, b=4));
    try_func(() ==> foo(x=1, y=2, a=3, b=4));
    try_func(() ==> foo(a=1));
    try_func(() ==> foo(a=1, b=2, 1));
    try_func(() ==> takes_one_named(1));
    try_func(() ==> takes_one_named(a=1));
    try_func(() ==> takes_one_named(z=1));
    try_func(() ==> takes_one_named_one_named_optional());
    try_func(() ==> takes_one_named_one_named_optional(y = 2));
}

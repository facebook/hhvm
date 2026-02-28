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

<<__EntryPoint>>
function main() {
    try_func(() ==> foo(x=1, 2, a=3, 4));
    try_func(() ==> foo(1, 2, a=3, c=4));
    try_func(() ==> foo(missing="", zz=1));
    // Prologue check reordering make it so we always report on the lexicographically
    // earliest name mismatch here.
    try_func(() ==> foo(zz=1, missing=""));
}

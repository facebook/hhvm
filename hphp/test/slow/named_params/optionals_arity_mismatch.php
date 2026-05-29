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

function foo(int $x, int $y, named int $a, named int $b = 4, named int $c = 5, int ...$args) {
    $v = vec[];
    $v[] = $x;
    $v[] = $y;
    $v[] = $a;
    $v[] = $b;
    $v[] = $c;
    foreach ($args as $arg) {
        $v[] = $arg;
    }
    var_dump($v);
}

<<__EntryPoint>>
function main() {
    try_func(() ==> foo(1, 2));
    try_func(() ==> foo(1, 2, b=1));
    try_func(() ==> foo(1, 2, b=1, c=2));
    try_func(() ==> foo(1, 2, 3));
    try_func(() ==> foo(1, a=1));
}

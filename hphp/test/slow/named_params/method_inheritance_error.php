<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

class C {
    public function foo(int $x, int $y, named int $a, named int $b) {
        var_dump(vec[$x, $y, $a, $b]);
    }
}

class D extends C {
    <<__Override>>
    public function foo(named int $x, named int $y, int $a, int $b) {
        var_dump(vec[$x + 100, $y + 100, $a + 100, $b +100]);
    }
}

function try_func($f) :mixed{
  try {
    $f();
    echo "Function did not throw\n";
  } catch (Exception $e) {
    printf("Caught %s: %s\n", get_class($e), $e->getMessage());
  }
}

function dispatch(C $c) {
    try_func(() ==> $c->foo(1, 2, a=3, b=4));
    try_func(() ==> $c->foo(x=1, y=2, 3, 4));
}

<<__EntryPoint>>
function main() {
    dispatch(new C());
    dispatch(new D());
}

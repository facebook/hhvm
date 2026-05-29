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

class C {
  public function foo(int $x, int $y, named int $a, named int $b) {}
  public function takes_one_named(named int $x) {}
  public function takes_no_named(int $x) {}
}

class D extends C {
  <<__Override>>
  public function foo(int $x, int $y, named int $a, named int $b) {}
  <<__Override>>
  public function takes_one_named(named int $x) {}
  <<__Override>>
  public function takes_no_named(int $x) {}
}

function dispatch(C $c) {
  try_func(() ==> $c->foo(1, 2, a=3));
  try_func(() ==> $c->foo(1, 2, a=3, b=4, c=5));
  try_func(() ==> $c->foo(1, 2, b=4));
  try_func(() ==> $c->foo(x=1, y=2, a=3, b=4));
  try_func(() ==> $c->foo(a=1));
  try_func(() ==> $c->foo(a=1, b=2, 1));
  try_func(() ==> $c->takes_one_named(1));
  try_func(() ==> $c->takes_no_named(x=0));
}

<<__EntryPoint>>
function main() {
  dispatch(new C());
  dispatch(new D());
}

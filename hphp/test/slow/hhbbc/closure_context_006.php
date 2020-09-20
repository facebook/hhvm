<?hh

class A {
  public function foo() {
    $x = () ==> {
      \HH\global_set('heh', $this);
      yield 1;
      yield 2;
      yield 3;
    };
    return $x;
  }
}

function main() {
  $x = new A();
  $x = $x->foo();
  foreach ($x() as $k) var_dump($k);
}

<<__EntryPoint>>
function main_closure_context_006() {
main();
}

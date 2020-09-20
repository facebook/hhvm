<?hh

class A {
  private $foo = 2;

  public async function genthings() {
    return () ==> () ==> () ==> () ==> () ==> () ==> {
      yield () ==> $this->foo = "2";
      yield () ==> $this->foo = "3";
      yield () ==> $this->foo = "4";
      yield () ==> $this->foo = "5";
    };
  }

  public function get() { return $this->foo; }
}

async function main() {
  $obj = new A();
  $x = await $obj->genthings();

  var_dump($obj->get());
  while ($x is Closure) {
    $x = $x();
    var_dump($obj->get());
  }
  foreach ($x as $v) {
    $v();
    var_dump($obj->get());
  }
  var_dump($obj->get());
}

<<__EntryPoint>>
function main_closure_context_007() {
main();
}

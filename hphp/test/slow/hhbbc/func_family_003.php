<?hh

abstract class Base {
  abstract public function foo();
}

class D1 extends Base {
  public function foo() {
    return "heh";
  }
}
class D2 extends Base {
  public function foo() {
    return (new D1)->foo();
  }
}

function main(Base $b) {
  $x = $b->foo();
  var_dump($x);
}



<<__EntryPoint>>
function main_func_family_003() {
main(new D1);
main(new D2);
}

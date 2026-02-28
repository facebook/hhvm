<?hh

abstract class Base {
  abstract public function foo():mixed;
}

class D1 extends Base {
  public function foo() :mixed{
    return "heh";
  }
}
class D2 extends Base {
  public function foo() :mixed{
    return (new D1)->foo();
  }
}

function main(Base $b) :mixed{
  $x = $b->foo();
  var_dump($x);
}



<<__EntryPoint>>
function main_func_family_003() :mixed{
main(new D1);
main(new D2);
}

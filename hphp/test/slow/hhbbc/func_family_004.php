<?hh

abstract class Base {
  abstract public function foo():mixed;
}

class D1 extends Base {
  public function foo() :mixed{ throw new Exception('heh'); }
}
class D2 extends Base {
  public function foo() :mixed{ return "this is a string\n"; }
}

function main(Base $b) :mixed{
  $x = $b->foo();
  var_dump($x);
}



<<__EntryPoint>>
function main_func_family_004() :mixed{
try {
  main(new D2);
  main(new D1);
} catch (Exception $x) {}
}

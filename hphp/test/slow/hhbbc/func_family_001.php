<?hh

class Heh {
  public function say() :mixed{ echo "hi\n"; }
}

abstract class Base {
  abstract public function foo():mixed;
}

class D1 extends Base { public function foo() :mixed{ return new Heh(); } }
class D2 extends Base { public function foo() :mixed{ return new Heh(); } }

function main(Base $b) :mixed{
  $x = $b->foo();
  $x->say();
}



<<__EntryPoint>>
function main_func_family_001() :mixed{
main(new D1);
main(new D2);
}

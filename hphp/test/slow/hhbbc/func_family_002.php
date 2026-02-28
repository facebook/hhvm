<?hh

class Heh {
  public function say() :mixed{ echo "hi\n"; }
}

abstract class RetBase {
  abstract public function get():mixed;
}

class RetD1 extends RetBase {
  public function get() :mixed{ return new Heh(); }
}

class RetD2 extends RetBase {
  public function get() :mixed{ return new Heh(); }
}

abstract class Base {
  abstract public function foo():mixed;
}

class D1 extends Base { public function foo() :mixed{ return new RetD1(); } }
class D2 extends Base { public function foo() :mixed{ return new RetD2(); } }

function main(Base $b) :mixed{
  $x = $b->foo();
  $x = $x->get();
  $x->say();
}



<<__EntryPoint>>
function main_func_family_002() :mixed{
main(new D1);
main(new D2);
}

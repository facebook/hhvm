<?hh

interface I {
  public function foo(inout int $x): void;
}

class C implements I {
  public function foo(inout int $x): void {}
}

abstract class B {
  public abstract function foo(inout int $x): void;
}

class D extends B {
  public function foo(inout int $x): void {}
}


<<__EntryPoint>>
function main_abstract_method() :mixed{
echo "Done.\n";
}

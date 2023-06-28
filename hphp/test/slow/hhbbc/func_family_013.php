<?hh

interface I {}

class A implements I {
  public function foo() :mixed{ return 1; }
}
class B implements I {
  public function foo() :mixed{ return 2; }
}
class C implements I {
  public function foo() :mixed{ return 3; }
}

interface I2 {}

class A2 implements I2 {
  public function foo() :mixed{ return 1; }
}

class P1 {
  private function foo() :mixed{ return 4; }
}

class B2 extends P1 implements I2 {
  public function foo() :mixed{ return 2; }
}
class C2 implements I2 {
  public function foo() :mixed{ return 3; }
}

<<__EntryPoint>>
function main() :mixed{
  echo "Done\n";
}

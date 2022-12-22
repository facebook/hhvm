<?hh

interface I {}

class A implements I {
  public function foo() { return 1; }
}
class B implements I {
  public function foo() { return 2; }
}
class C implements I {
  public function foo() { return 3; }
}

interface I2 {}

class A2 implements I2 {
  public function foo() { return 1; }
}

class P1 {
  private function foo() { return 4; }
}

class B2 extends P1 implements I2 {
  public function foo() { return 2; }
}
class C2 implements I2 {
  public function foo() { return 3; }
}

<<__EntryPoint>>
function main() {
  echo "Done\n";
}

<?hh

class A {
  public function foo(string $bar) {
  }
}

class B extends A {
  public function foo($bar) {
  }
}

abstract class C {
  abstract public function foo(string $bar);
}

class D extends C {
  public function foo($bar) {
  }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }

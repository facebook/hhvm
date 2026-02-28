<?hh

class A {
  public function foo(string $bar) :mixed{
  }
}

class B extends A {
  public function foo($bar) :mixed{
  }
}

abstract class C {
  abstract public function foo(string $bar):mixed;
}

class D extends C {
  public function foo($bar) :mixed{
  }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }

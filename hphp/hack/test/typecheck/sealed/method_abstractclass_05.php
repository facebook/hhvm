<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

abstract class C {
  <<__Sealed(T1::class)>>
  abstract public function foo(): void;
}

trait T1 {
  <<__Override>>
  public function foo(): void { echo "I am foo in T1\n"; }
}

trait T2 {
  <<__Override>>
  public function foo(): void { echo "I am foo in T2\n"; }
}

class D1 extends C {
  use T1;  // this is ok
}

class D2 extends C {
  use T2;  // this is rejected
}

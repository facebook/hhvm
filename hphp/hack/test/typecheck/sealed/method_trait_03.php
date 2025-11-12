<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

trait T1 {
  <<__Sealed(C::class, T2::class)>>
  public function foo(): void {
    echo "I am foo in T1\n";
  }
}

class C {}
trait T2 {}

class E1 {
  use T1;
}

class E2 extends E1 {
  // this is rejected
  <<__Override>>
  public function foo(): void {
    echo "I am foo in E2\n";
  }
}

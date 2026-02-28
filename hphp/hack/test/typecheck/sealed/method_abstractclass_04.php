<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

abstract class C {
  <<__Sealed(T::class)>>
  abstract public function foo(): void;
}

trait T {
  <<__Override>>
  public function foo(): void {}
}

class D extends C {
  use T;
  // this should be rejected
  <<__Override>>
  public function foo(): void {
    echo "I am foo in D\n";
  }
}

class E extends C {
  // this should be rejected
  <<__Override>>
  public function foo(): void {
    echo "I am foo in E\n";
  }
}

<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

trait T1 {
  <<__Sealed(C::class, T2::class)>>
  public function foo(): void {
    echo "I am foo in T1\n";
  }
}

// class C can override foo in T1
class C {
  use T1;

  <<__Override>>
  public function foo(): void {
    echo "I am foo in C\n";
  }
}

class E extends C {
  <<__Override>>
  public function foo(): void {
    echo "I am foo in D\n";
  }
}

// trait T2 can override foo in T1, T2 can then be used by arbitrary classes
trait T2 {
  use T1;

  <<__Override>>
  public function foo(): void {
    echo "I am foo in T2\n";
  }
}

class D {
  use T2;
}

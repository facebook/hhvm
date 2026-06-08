<?hh

trait T1 {
  <<__Sealed(C::class, T2::class)>>
  public function foo(): void {
    echo "I am foo in T1\n";
  }
}

class C {
  // warning as C does not use T1
}
trait T2 {
  // warning as T2 does not use T1
}

// class D cannot override foo in T1
class D {
  use T1;

  <<__Override>>
  public function foo(): void {
    echo "I am foo in D\n";
  }
}

// trait T3 cannot override foo in T1
trait T3 {
  use T1;

  <<__Override>>
  public function foo(): void {
    echo "I am foo in T3\n";
  }
}

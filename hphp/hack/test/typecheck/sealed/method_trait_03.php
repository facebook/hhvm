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

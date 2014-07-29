<?hh

class C1 {
  public function meth(...$args): void {}
}

class C2 extends C1 {
  public function meth($x = null): void {}
}

<?hh

class C {
  <<__Sealed(D::class)>>
  public function foo(): void {}
}

class D {}

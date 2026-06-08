<?hh

class C {
  <<__Sealed(D::class)>>
  public function foo(): void {}
}

class D extends C {
  // warning here, as D is expected to override foo
}

class E extends D {
  // this should be rejected
  <<__Override>>
  public function foo(): void {}
}

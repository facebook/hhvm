<?hh

class A {
  public function c(): void {}
}

function f(): varray<(function (A): void)> {
  return vec[
    ($f) ==> { $f->c(); }
  ];
}

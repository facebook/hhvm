<?hh

class A {
  public function c(): void {}
}

function f(): varray<(function (A): void)> {
  return varray[
    ($f) ==> { $f->c(); }
  ];
}

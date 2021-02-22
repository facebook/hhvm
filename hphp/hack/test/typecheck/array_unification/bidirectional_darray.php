<?hh

class A {
  public function c(): void {}
}

function f(): darray<string, (function (A): void)> {
  return darray[
    'hi' => ($f) ==> { $f->c(); }
  ];
}

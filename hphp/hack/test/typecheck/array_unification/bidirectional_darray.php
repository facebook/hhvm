<?hh

class A {
  public function c(): void {}
}

function f(): darray<string, (function (A): void)> {
  return dict[
    'hi' => ($f) ==> { $f->c(); }
  ];
}

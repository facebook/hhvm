<?hh

class A {
  public function c(): void {}
}

function f(): ~vec<(function (A): void)> {
  return vec[
    ($f) ==> { $f->c(); }
  ];
}
function g(): ~dict<string, (function (A): void)> {
  return dict[
    'hi' => ($f) ==> { $f->c(); }
  ];
}

function ff(): ~Traversable<(function (A): void)> {
  return vec[
    ($f) ==> { $f->c(); }
  ];
}
function gg(): ~KeyedTraversable<string, (function (A): void)> {
  return dict[
    'hi' => ($f) ==> { $f->c(); }
  ];
}

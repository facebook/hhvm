<?hh

class A {
  public function c(): void {}
}

<<__SupportDynamicType>>
function f(): ~vec<(function (A): void)> {
  return vec[
    ($f) ==> { $f->c(); }
  ];
}
<<__SupportDynamicType>>
function g(): ~dict<string, (function (A): void)> {
  return dict[
    'hi' => ($f) ==> { $f->c(); }
  ];
}

<<__SupportDynamicType>>
function ff(): ~Traversable<(function (A): void)> {
  return vec[
    ($f) ==> { $f->c(); }
  ];
}
<<__SupportDynamicType>>
function gg(): ~KeyedTraversable<string, (function (A): void)> {
  return dict[
    'hi' => ($f) ==> { $f->c(); }
  ];
}

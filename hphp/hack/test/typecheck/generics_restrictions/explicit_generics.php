<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function toplevel1<<<__Explicit>> T>(T $x): void {}
function toplevel2<T1, <<__Explicit>> T2>(T1 $x, T2 $y): void {}
function bar(dynamic $m): void {
  toplevel1(3);
  toplevel2(2, 3);
  toplevel2<_, string>(3, "a");
  toplevel2<_, _>(3, "a");
  toplevel2<_, vec<_>>(3, vec["A"]);
  $x = HH\FIXME\UNSAFE_CAST<dynamic, vec<_>>($m, 'blah');
}

class C {
  public function meth1<<<__Explicit>> T>(T $x): void {}
  public function meth2<T1, <<__Explicit>> T2>(T1 $x, T2 $y): void {}
  public function barmeth(): void {
    $this->meth1(3);
    $this->meth2(2, 3);
    $this->meth2<_, string>(3, "a");
    $this->meth2<_, _>(3, "a");
    $this->meth2<_, vec<_>>(3, vec["A"]);
  }
}

class G<<<__Explicit>> T> {

}
function testnew(): void {
  $g1 = new G();
  $g2 = new G<_>();
  $g3 = new G<vec<_>>();
}

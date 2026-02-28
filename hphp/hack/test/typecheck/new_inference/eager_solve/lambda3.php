<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Doer<-T> {
  public function __construct(private Action<T> $act) { }
  public function doit(T $x):void {
    $a = $this->act;
    $a($x);
  }
}

class C {
  public function foo():void { }
}
type Action<-T> = (function(T):void);
function apply_and_make<T>(Action<T> $f, T $x):Doer<T> {
  ($f)($x);
  return new Doer($f);
}

function testit(C $x):void {
  $y = apply_and_make($c ==> { $c->foo(); }, $x);
}

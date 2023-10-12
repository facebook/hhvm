<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():void { }
}

function my_array_map<T1,T2>((function(T1):T2) $f, vec<T1> $v): vec<T2> {
  throw new Exception();
}

function testit():void {
  $x = vec[new C()];
  $y = my_array_map($c ==> $c, $x);
  $y[0]->foo();
}

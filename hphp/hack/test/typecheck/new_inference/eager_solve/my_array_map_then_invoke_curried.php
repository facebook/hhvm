<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():void { }
}

function my_array_map<T1,T2>(vec<T1> $v): (function((function(T1):T2)): vec<T2>) {
  throw new Exception();
}

function testit():void {
  $x = vec[new C()];
  $y = my_array_map($x)($c ==> $c);
  $y[0]->foo();
}

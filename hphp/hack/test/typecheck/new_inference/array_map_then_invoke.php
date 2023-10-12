<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():void { }
}

function my_array_map<Tu,Tv>((function(Tu):Tv) $f, vec<Tu> $v): vec<Tv> {
  return vec[];
}
function testit():void {
  $x = vec[new C()];
  // Tv appears covariantly
  // Tu appears covariantly
  $y = my_array_map($c ==> $c, $x);
  $y[0]->foo();
}

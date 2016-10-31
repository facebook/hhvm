<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test_get<T as Indexish<string,int>>(T $x):int {
  return $x['a'];
}

interface J {
  public function getName():string;
}
function test_get_multi<T as Indexish<string,int> as J>(T $x): int {
  $y = $x->getName();
  return $x[$y];
}

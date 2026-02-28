<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface I {
  public function bar():void;
}
type S = supportdyn<shape('a' => supportdyn<(function(I):void)>)>;
function foo():S {
  return shape('a' => $x ==> $x->bar());
}

function foo2():supportdyn<(function(I):void)> {
  return $x ==> $x->bar();
}

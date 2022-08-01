<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<file:__EnableUnstableFeatures('upcast_expression')>>

class C {
  public dynamic $d = 3 upcast dynamic;
}

<<__EntryPoint>>
function main():void {
  $x = new C();
  var_dump($x->d);
}

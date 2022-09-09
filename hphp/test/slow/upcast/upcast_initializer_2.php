<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<file:__EnableUnstableFeatures('upcast_expression')>>

class C {
  public dynamic $d = __FILE__ upcast dynamic;
}

<<__EntryPoint>>
function main():void {
  $x = new C();
  var_dump($x->d);
}

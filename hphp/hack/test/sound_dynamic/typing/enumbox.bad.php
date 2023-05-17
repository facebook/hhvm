<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<file:__EnableUnstableFeatures('upcast_expression')>>

enum E : int as int { A = 3; }
enum F : int as int { B = 5; }

<<__SupportDynamicType>>
class EBox {
  public function __construct(public E $item) { }
}

<<__EntryPoint>>
function main():void {
  $d = new EBox(E::A);
  $dd = $d upcast dynamic;
  $dd->item = F::B;
  $x = $d->item;
  // Now I think I've got an E but I've actually got an F
}

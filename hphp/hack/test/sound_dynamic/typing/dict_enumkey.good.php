<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

enum E: int as int {
  A = 3;
}

<<__SupportDynamicType>>
function getDict():dict<E,string> {
  return dict[];
}

<<__SupportDynamicType>>
interface I {
  public function getKey():int;
}


<<__SupportDynamicType>>
class C implements I {
public function getKey():(~E & int) {
  return E::A;
  }
}

<<__SupportDynamicType>>
function testit():void {
  $c = new C();
  $y = getDict();
  $z = $c->getKey();
  $x = $y[$z];
}

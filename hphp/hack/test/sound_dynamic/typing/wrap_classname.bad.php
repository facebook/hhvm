<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class Wrap {
  public function __construct(private vec<int> $vi) { }
  public function get():vec<int> { return $this->vi; }
}

function getVecInt():~vec<int> {
  return vec["a"] as dynamic;
  }

function breakit(dynamic $d):int {
  $x = new $d(getVecInt());
  $x as Wrap;
  return $x->get()[0];
}

<<__EntryPoint>>
function main():void {
  $c = Wrap::class;
  breakit($c);
}

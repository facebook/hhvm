<?hh

<<__SupportDynamicType>>
class Box {
  private vec<int> $vi;
  public function __construct(vec<int> $vi) {
    $this->vi = $vi;
  }
  public function getVecInt(): vec<int> {
    return $this->vi;
  }
}

function getVecInt():~vec<int> {
  return vec["a"] as dynamic;
}

<<__EntryPoint>>
function test():int {
  $x = new Box(getVecInt());
  $vi = $x->getVecInt();
  // Shows vec<int>. This is a lie!
  // hh_show($vi);
  return $vi[0];
}

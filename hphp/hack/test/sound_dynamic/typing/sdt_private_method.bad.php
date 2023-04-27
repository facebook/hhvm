<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

class A {
  const type TC = int;
}

<<__SupportDynamicType>>
class C {
  private A::TC $prop = 5;
  private function set(A::TC $vi):void {
    // This should be rejected
    $this->prop = $vi;
  }
  public function breakit(dynamic $d):void {
    $d->set("A");
  }
  public function get():int {
    return $this->prop;
  }
}

<<__EntryPoint>>
function main():void {
  $c = new C();
  $c->breakit($c);
  $x = $c->get();
  }

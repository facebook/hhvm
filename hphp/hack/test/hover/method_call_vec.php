<?hh

class FooParent {
  public function bar(): void {}
}

function getVec():vec<FooParent> {
  return vec[];
}
class Foo extends FooParent {
  public function callIt(): void {
    $v = getVec();
    $v[0]->bar();
    //     ^ hover-at-caret
  }
}

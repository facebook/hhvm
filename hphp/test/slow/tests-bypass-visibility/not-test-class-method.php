<?hh

class BestObject {
  <<__TestsBypassVisibility>>
  private function foo(): void { var_dump("foo"); }
}

class NotATest {
  public function test(): void {
    $obj = new BestObject();
    $obj->foo();
  }
}

<<__EntryPoint>>
function main(): void {
  (new NotATest())->test();
}

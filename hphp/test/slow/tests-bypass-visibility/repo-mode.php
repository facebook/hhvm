<?hh

class BestObject {
  <<__TestsBypassVisibility>>
  private function foo(): void { var_dump("foo"); }
}

class WWWTest {}

class FooTest extends WWWTest {
  public function test(): void {
    $obj = new BestObject();
    $obj->foo();
  }
}

<<__EntryPoint>>
function main(): void {
  (new FooTest())->test();
}

<?hh

class BestObject {
  private function quux(): void { var_dump("quux"); }
}

class WWWTest {}

class FooTest extends WWWTest {
  public function test(): void {
    $obj = new BestObject();
    $obj->quux();
  }
}

<<__EntryPoint>>
function main(): void {
  (new FooTest())->test();
}

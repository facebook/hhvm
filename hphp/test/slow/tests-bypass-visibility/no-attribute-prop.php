<?hh

class BestObject {
  private int $secret = 42;
}

class WWWTest {}

class FooTest extends WWWTest {
  public function test(): void {
    $obj = new BestObject();
    var_dump($obj->secret);
  }
}

<<__EntryPoint>>
function main(): void {
  (new FooTest())->test();
}

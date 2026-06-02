<?hh

class BestObject {
  private static int $secret = 42;
}

class WWWTest {}

class FooTest extends WWWTest {
  public function test(): void {
    var_dump(BestObject::$secret);
  }
}

<<__EntryPoint>>
function main(): void {
  (new FooTest())->test();
}

<?hh

class WWWTest {}

class ClosureTarget {
  <<__TestsBypassVisibility>>
  private function secret(): int {
    return 42;
  }

  <<__TestsBypassVisibility>>
  private int $secretProp = 7;
}

class CopiedClosureTest extends WWWTest {
  public function test(ClosureTarget $obj): void {
    $method = () ==> $obj->secret();
    echo $method()."\n";

    $prop = () ==> $obj->secretProp;
    echo $prop()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  (new CopiedClosureTest())->test(new ClosureTarget());
}

<?hh

class WWWTest {}

class ClosureTarget {
  <<__TestsBypassVisibility>>
  private function secret(): int { return 42; }

  <<__TestsBypassVisibility>>
  private int $secret_prop = 0;
}

class ClosureTest extends WWWTest {
  public function test(ClosureTarget $obj): void {
    // Can a closure capture and call the bypass-visibility method?
    $fn = () ==> $obj->secret();
    $_ = $fn();

    // Can a closure access bypass-visibility property?
    $fn2 = () ==> $obj->secret_prop;
    $_ = $fn2();
  }
}

<?hh

class WWWTest {}

class PrivateCtor {
  <<__TestsBypassVisibility>>
  private function __construct(private int $x) {}
}

class CtorTest extends WWWTest {
  public function test(): void {
    // Tests can directly construct a class with a bypassable private constructor.
    $obj = new PrivateCtor(1);
  }
}

class CtorNonTest {
  public function test(): void {
    $obj = new PrivateCtor(1); // error
  }
}

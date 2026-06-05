<?hh

class WWWTest {}

// Trait with require class constraint and bypass visibility
trait ReqTrait {
  require class ReqTarget;

  <<__TestsBypassVisibility>>
  private function trait_secret(): void {}
}

final class ReqTarget {
  use ReqTrait;
}

class ReqTest extends WWWTest {
  public function test(ReqTarget $obj): void {
    $obj->trait_secret(); // ok: bypass in test
  }
}

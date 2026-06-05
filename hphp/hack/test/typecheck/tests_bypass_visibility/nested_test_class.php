<?hh

class WWWTest {}

class Target {
  <<__TestsBypassVisibility>>
  private function secret(): void {}
}

// Direct subclass of WWWTest
class DirectTest extends WWWTest {
  public function test(Target $t): void {
    $t->secret(); // ok
  }
}

// Indirect subclass of WWWTest (extends through another test class)
class IndirectTest extends DirectTest {
  public function test2(Target $t): void {
    $t->secret(); // should also be ok - transitive subclass of WWWTest
  }
}

// Helper class used by test but not extending WWWTest
class TestHelper {
  public function help(Target $t): void {
    $t->secret(); // error: not a test class
  }
}

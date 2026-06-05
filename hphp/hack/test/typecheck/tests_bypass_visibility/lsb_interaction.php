<?hh

class WWWTest {}

class LsbParent {
  <<__LSB, __TestsBypassVisibility>>
  private static int $lsb_prop = 0;

  <<__TestsBypassVisibility>>
  private static int $non_lsb_prop = 0;
}

class LsbChild extends LsbParent {}

class LsbTest extends WWWTest {
  public function test(): void {
    // LSB property: already inherited by subclasses, but private.
    // With bypass visibility, should be accessible from tests.
    $_ = LsbParent::$lsb_prop;
    $_ = LsbChild::$lsb_prop; // LSB: each class has own copy

    // Non-LSB private static: inherited via bypass visibility
    $_ = LsbParent::$non_lsb_prop;
    $_ = LsbChild::$non_lsb_prop;
  }
}

<?hh

class WWWTest {}

class StaticPropertyTarget {
  <<__TestsBypassVisibility>>
  private static int $priv_prop = 0;

  <<__TestsBypassVisibility>>
  protected static int $prot_prop = 0;

  <<__TestsBypassVisibility>>
  private static function priv_method(): void {}
}

class StaticPropertyTest extends WWWTest {
  public function test(): void {
    StaticPropertyTarget::priv_method(); // ok: static methods use the main gate
    $_ = StaticPropertyTarget::$priv_prop; // error: static-property gate is off
    $_ = StaticPropertyTarget::$prot_prop; // error: static-property gate is off
  }
}

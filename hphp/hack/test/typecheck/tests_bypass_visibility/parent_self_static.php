<?hh

class WWWTest {}

class Base2 extends WWWTest {
  <<__TestsBypassVisibility>>
  private static int $base_priv_prop = 0;

  <<__TestsBypassVisibility>>
  private static function base_priv(): void {}

  <<__TestsBypassVisibility>>
  protected static function base_prot(): void {}

  public function test_self_and_static(): void {
    self::base_priv();
    static::base_priv();
    self::base_prot();
    static::base_prot();

    $_ = self::$base_priv_prop;
    $_ = static::$base_priv_prop;
  }
}

class Child2 extends Base2 {
  public function test_parent_self_and_static(): void {
    parent::base_priv();
    self::base_priv();
    static::base_priv();
    parent::base_prot();
    self::base_prot();
    static::base_prot();

    $_ = parent::$base_priv_prop;
    $_ = self::$base_priv_prop;
    $_ = static::$base_priv_prop;
  }
}

class ExplicitClassTest extends WWWTest {
  public function test(): void {
    Base2::base_priv();
    Base2::base_prot();
    Child2::base_prot();
  }
}

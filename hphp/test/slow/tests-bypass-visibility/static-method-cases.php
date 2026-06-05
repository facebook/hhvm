<?hh

class WWWTest {}

class StaticMethodBase extends WWWTest {
  <<__TestsBypassVisibility>>
  private static function basePriv(): string {
    return 'base_priv';
  }

  <<__TestsBypassVisibility>>
  protected static function baseProt(): string {
    return 'base_prot';
  }

  public function testSelfAndStatic(): void {
    echo self::basePriv()."\n";
    echo static::basePriv()."\n";
    echo self::baseProt()."\n";
    echo static::baseProt()."\n";
  }
}

class StaticMethodChild extends StaticMethodBase {
  public function testParentSelfAndStatic(): void {
    echo parent::basePriv()."\n";
    echo self::basePriv()."\n";
    echo static::basePriv()."\n";
    echo parent::baseProt()."\n";
    echo self::baseProt()."\n";
    echo static::baseProt()."\n";
  }
}

class ExplicitStaticMethodTest extends WWWTest {
  public function test(): void {
    echo StaticMethodBase::basePriv()."\n";
    echo StaticMethodBase::baseProt()."\n";
    echo StaticMethodChild::baseProt()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  (new StaticMethodBase())->testSelfAndStatic();
  (new StaticMethodChild())->testParentSelfAndStatic();
  (new ExplicitStaticMethodTest())->test();
}

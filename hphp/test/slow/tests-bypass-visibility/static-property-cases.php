<?hh

class WWWTest {}

class StaticPropertyBase extends WWWTest {
  <<__TestsBypassVisibility>>
  private static int $basePrivProp = 10;

  public function testSelfAndStatic(): void {
    echo self::$basePrivProp."\n";
    echo static::$basePrivProp."\n";
  }
}

class StaticPropertyChild extends StaticPropertyBase {
  public function testParent(): void {
    echo parent::$basePrivProp."\n";
  }
}

class LsbPropertyParent {
  <<__LSB, __TestsBypassVisibility>>
  private static int $lsbProp = 1;

  <<__TestsBypassVisibility>>
  private static int $nonLsbProp = 2;
}

class LsbPropertyChild extends LsbPropertyParent {}

class CopiedStaticPropertyTest extends WWWTest {
  public function test(): void {
    echo LsbPropertyParent::$lsbProp."\n";
    echo LsbPropertyChild::$lsbProp."\n";
    echo LsbPropertyParent::$nonLsbProp."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  (new StaticPropertyBase())->testSelfAndStatic();
  (new StaticPropertyChild())->testParent();
  (new CopiedStaticPropertyTest())->test();
}

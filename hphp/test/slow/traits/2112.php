<?hh

trait T {
  private static function blarg() :mixed{
    echo "T::blarg\n";
  }
  private static function test() :mixed{
    echo __CLASS__ . "\n";
    echo static::class . "\n";
    self::blarg();
  }
  public static function doTest() :mixed{
    self::test();
  }
}
class C {
  use T;
}
class D extends C {
  protected static function test() :mixed{
    echo "D::test\n";
  }
  public static function doTest() :mixed{
    parent::doTest();
  }
}

<<__EntryPoint>>
function main_2112() :mixed{
D::doTest();
echo "Done\n";
}

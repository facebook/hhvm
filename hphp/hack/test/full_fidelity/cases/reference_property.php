<?hh

class TestClass {
  public int $prop = 42;
  public static int $staticProp = 42;
}

function test(TestClass $test_class) : void {
  bar(&test_class->prop);
  bar(&test_class?->prop);
  bar(&test_class::$prop);
}

function bar(&$x) : void {
}

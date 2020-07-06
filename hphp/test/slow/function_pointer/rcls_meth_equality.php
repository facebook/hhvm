<?hh

class TestBase {
  public static function foo<reify T>(): void {}

  public static function bar<reify T>(): void {}
}

class TestComparison {
  public static function foo<reify T>(): void {}
}

class ChildClass extends TestBase {
  public static function get_parent_foo() {
    return parent::foo<int>;
  }

  public static function get_static_foo() {
    return static::foo<int>;
  }

  public static function get_self_foo() {
    return self::foo<int>;
  }
}

function wrap($fun) {
  try {
    $fun();
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

function comp($x, $y, $case) {
  print($case."\n");
  wrap(() ==> var_dump($x == $y));
  wrap(() ==> var_dump($x === $y));
  wrap(() ==> var_dump($x != $y));
  wrap(() ==> var_dump($x !== $y));
  print("\n");
}

<<__EntryPoint>>
function main(): void {
  comp(TestBase::foo<int>, TestBase::foo<int>, "Same");

  comp(TestBase::foo<int>, TestBase::foo<string>, "Different generic");

  comp(TestBase::foo<int>, TestBase::bar<int>, "Different method");

  comp(TestBase::foo<int>, TestComparison::foo<int>, "Different class");

  comp(TestBase::foo<int>, ChildClass::foo<int>, "Child class");

  comp(TestBase::foo<int>, ChildClass::get_parent_foo(), "via parent::");

  comp(ChildClass::foo<int>, ChildClass::get_self_foo(), "via self::");

  comp(TestBase::foo<int>, ChildClass::get_self_foo(), "self:: on child class");

  comp(ChildClass::foo<int>, ChildClass::get_static_foo(), "via static::");

  comp(TestBase::foo<int>, ChildClass::get_static_foo(), "static:: on child class");

  comp(TestBase::foo<int>, 'TestBase::foo', "String");

  comp(TestBase::foo<int>, class_meth(TestBase::class, 'foo'), "Regular class_meth");

  comp(TestBase::foo<int>, true, "Boolean");

  comp(TestBase::foo<int>, 1, "Int");

  comp(TestBase::foo<int>, vec['TestBase', 'foo'], "Vec");
}

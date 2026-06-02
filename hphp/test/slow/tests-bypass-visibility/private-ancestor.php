<?hh

class WWWTest {}

// Scenario 1: Bypass works when there is no private ancestor conflict.
// A test class invokes a protected method marked with __TestsBypassVisibility
// on an unrelated target. The bypass should grant access.
class Unrelated1 {
  <<__TestsBypassVisibility>>
  protected function bar(): mixed { return "Unrelated1::bar"; }
}

class Test1 extends WWWTest {
  public function test(): void {
    $t = new Unrelated1();
    echo $t->bar() . "\n";
  }
}

// Scenario 2: A private ancestor should take priority over test bypass.
// Test2 defines a private bar(). Its subclass Intermediate2 redeclares bar()
// as protected with __TestsBypassVisibility. When Test2 calls $o->bar() on a
// Bottom2 instance, dispatch should resolve to Test2's private bar().
class Test2 extends WWWTest {
  private function bar(): mixed { return "Test2::bar"; }
  public function test(): void {
    $o = new Bottom2();
    echo $o->bar() . "\n";
  }
}

class Intermediate2 extends Test2 {
  <<__TestsBypassVisibility>>
  protected function bar(): mixed { return "Intermediate2::bar"; }
}

class Bottom2 extends Intermediate2 {}

// Scenario 3: Like scenario 2 but with additional inheritance below the ctx.
// Verifies the behaviour holds when the object is further down the chain.
class Test3 extends WWWTest {
  private function bar(): mixed { return "Test3::bar"; }
  public function test(): void {
    $o = new Deep3();
    echo $o->bar() . "\n";
  }
}

class Intermediate3 extends Test3 {
  <<__TestsBypassVisibility>>
  protected function bar(): mixed { return "Intermediate3::bar"; }
}

class Sub3 extends Intermediate3 {}
class Deep3 extends Sub3 {}

<<__EntryPoint>>
function main(): void {
  (new Test1())->test();
  (new Test2())->test();
  (new Test3())->test();
}

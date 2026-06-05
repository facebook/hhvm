//// modules.php
<?hh

new module foo {}

//// foo.php
<?hh

module foo;

<<__TestsBypassVisibility>>
internal class InternalFoo {
  <<__TestsBypassVisibility>>
  internal function secret(): int { return 42; }

  public function pub(): void {}
}

//// test.php
<?hh

class WWWTest {}

class InternalClassTest extends WWWTest {
  public function test(): void {
    $f = new InternalFoo(); // ok: class has attribute, in test context
    $f->secret(); // ok: method has attribute, in test context
    $f->pub(); // ok: public method
  }
}

trait InternalClassTestTrait {
  require extends WWWTest;

  public function test_trait(): void {
    $f = new InternalFoo(); // ok: trait requirement makes this a test context
    $f->secret(); // ok: method has attribute, in test context
  }
}

class InternalClassTraitTest extends WWWTest {
  use InternalClassTestTrait;
}

//// non_test.php
<?hh

class NotATest2 {
  public function test(): void {
    $f = new InternalFoo(); // error: not in test context
  }
}

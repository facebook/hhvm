//// modules.php
<?hh

new module m {}

//// defs.php
<?hh

module m;

<<__TestsBypassVisibility>>
internal abstract class InternalAbstract {
  <<__TestsBypassVisibility>>
  internal abstract function secret(): int;
}

<<__TestsBypassVisibility>>
internal class InternalConcrete extends InternalAbstract {
  <<__Override, __TestsBypassVisibility>>
  internal function secret(): int { return 42; }
}

//// test.php
<?hh

class WWWTest {}

class InternalAbstractTest extends WWWTest {
  public function test(): void {
    // Reference internal abstract class as a type
    $obj = new InternalConcrete();
    $this->helper($obj);
  }

  private function helper(InternalAbstract $a): void {
    // Use internal abstract class in type position
    $_ = $a->secret();
  }
}

//// non_test.php
<?hh

class InternalAbstractNonTest {
  public function test(): void {
    $obj = new InternalConcrete(); // error: not in test context
  }
}

//// modules.php
<?hh

new module m {}

//// defs.php
<?hh

module m;

<<__TestsBypassVisibility>>
internal interface InternalIface {
  public function iface_method(): int;
}

<<__TestsBypassVisibility>>
internal class IfaceImpl implements InternalIface {
  public function iface_method(): int { return 1; }
}

//// test.php
<?hh

class WWWTest {}

class InternalIfaceTest extends WWWTest {
  public function test(): void {
    $obj = new IfaceImpl();
    // Reference internal interface as a type
    $this->helper($obj);
  }

  private function helper(InternalIface $i): void {
    $_ = $i->iface_method();
  }
}

//// non_test.php
<?hh

class InternalIfaceNonTest {
  public function test(): void {
    $obj = new IfaceImpl(); // error: not in test context
  }

  // error: referencing internal interface outside test context
  private function helper(InternalIface $i): void {}
}

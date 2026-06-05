//// modules.php
<?hh

new module pi {}

//// target.php
<?hh

module pi;

class ProtIntTarget {
  <<__TestsBypassVisibility>>
  protected internal function prot_int(): void {}
}

//// test.php
<?hh

class WWWTest {}

// Not in module pi, not a subclass -- both protected AND internal fail
// but the attribute should bypass both in test context
class ProtIntTest extends WWWTest {
  public function test(ProtIntTarget $obj): void {
    $obj->prot_int(); // ok: bypass both protected and internal
  }
}

//// non_test.php
<?hh

class ProtIntNonTest {
  public function test(ProtIntTarget $obj): void {
    $obj->prot_int(); // error: not in test context
  }
}

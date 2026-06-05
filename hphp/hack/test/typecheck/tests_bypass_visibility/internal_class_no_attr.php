//// modules.php
<?hh

new module m {}

//// defs.php
<?hh

module m;

internal class InternalNoAttr {
  public function pub(): void {}
}

//// test.php
<?hh

class WWWTest {}

class InternalClassNoAttrTest extends WWWTest {
  public function test(): void {
    new InternalNoAttr(); // error: class is internal, no attribute; in test context — suggest attribute
  }
}

//// non_test.php
<?hh

class InternalClassNoAttrNonTest {
  public function test(): void {
    new InternalNoAttr(); // error: not in test context — no suggestion
  }
}

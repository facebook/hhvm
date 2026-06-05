//// modules.php
<?hh

new module foo {}

//// foo.php
<?hh

module foo;

class InternalFoo {
  <<__TestsBypassVisibility>>
  internal function int_method(): void {}

  <<__TestsBypassVisibility>>
  internal static function int_static(): void {}

  internal function no_attr(): void {}
}

//// test.php
<?hh

class WWWTest {}

class InternalTest extends WWWTest {
  public function test(InternalFoo $f): void {
    $f->int_method(); // ok: bypasses internal
    InternalFoo::int_static(); // ok: bypasses internal static
    $f->no_attr(); // error: no attribute
  }
}

//// non_test.php
<?hh

class InternalNonTest {
  public function test(InternalFoo $f): void {
    $f->int_method(); // error: not in test context
    InternalFoo::int_static(); // error: not in test context
  }
}

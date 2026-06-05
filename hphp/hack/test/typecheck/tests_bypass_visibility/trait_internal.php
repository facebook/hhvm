//// modules.php
<?hh

new module m {}

//// trait.php
<?hh

module m;

<<__ModuleLevelTrait>>
trait ModTrait {
  <<__TestsBypassVisibility>>
  internal function int_meth(): void {}
}

//// user.php
<?hh

module m;

class TraitUser {
  use ModTrait;
}

//// test.php
<?hh

class WWWTest {}

class InternalTraitTest extends WWWTest {
  public function test(TraitUser $obj): void {
    $obj->int_meth(); // ok: bypass visibility in test
  }
}

//// non_test.php
<?hh

class InternalTraitNonTest {
  public function test(TraitUser $obj): void {
    $obj->int_meth(); // error: not in test context
  }
}

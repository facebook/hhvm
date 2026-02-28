////long/directory/name/class_def.php
<?hh
class Foo {
  public function f(int $x): void {}
}

////another/directory/treq_ext_foo.php
<?hh
trait TReqExtFoo {
  require extends Foo;
}

////child.php
<?hh
class Child extends Foo {
  public function f(string $x): void {}
}

////some/more/directories/treq_ext_child.php
<?hh
trait TReqExtChild {
  require extends Child;
}

////long/directory/name/again/tboth_traits.php
<?hh
trait TBothTraits {
  use TReqExtChild;
  use TReqExtFoo;
}

////long/directory/name/class_def.php
<?hh // strict
class Foo {
  public function f(int $x): void {}
}

////another/directory/treq_ext_foo.php
<?hh // strict
trait TReqExtFoo {
  require extends Foo;
}

////child.php
<?hh // strict
class Child extends Foo {
  public function f(string $x): void {}
}

////some/more/directories/treq_ext_child.php
<?hh // strict
trait TReqExtChild {
  require extends Child;
}

////long/directory/name/again/tboth_traits.php
<?hh // strict
trait TBothTraits {
  use TReqExtChild;
  use TReqExtFoo;
}

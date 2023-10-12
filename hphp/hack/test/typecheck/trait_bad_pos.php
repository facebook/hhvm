////foo.php
<?hh // strict
class Foo {
  public function f(int $x): void {}
}

////treq_ext_foo.php
<?hh // strict
trait TReqExtFoo {
  require extends Foo;
}

////child.php
<?hh // strict
class Child extends Foo {
  public function f(string $x): void {}
}

////treq_ext_child.php
<?hh // strict
trait TReqExtChild {
  require extends Child;
}

////tboth_traits.php
<?hh // strict
trait TBothTraits {
  use TReqExtChild;
  use TReqExtFoo;

  public function g(): void {
    $this->f(0);
  }
}

class A extends Child {
  use TBothTraits;
}

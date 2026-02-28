////foo.php
<?hh
class Foo {
  public function f(int $x): void {}
}

////treq_ext_foo.php
<?hh
trait TReqExtFoo {
  require extends Foo;
}

////child.php
<?hh
class Child extends Foo {
  public function f(string $x): void {}
}

////treq_ext_child.php
<?hh
trait TReqExtChild {
  require extends Child;
}

////tboth_traits.php
<?hh
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

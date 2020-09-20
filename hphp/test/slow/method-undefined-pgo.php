<?hh

abstract class BaseCls {
  public function base() {}
}

class HasFoo extends BaseCls {
  public function foo0() {}
  public function foo1() {}
}

class HasFooChild1 extends HasFoo {}

class HasFooChild2 extends HasFoo {}

class DoesNotHaveFoo extends BaseCls {
  public function bar() {}
}

function fn(BaseCls $x) {
  $x->foo1();
}

<<__EntryPoint>>
function main() {
  fn(new HasFooChild1());
  fn(new HasFooChild2());
  fn(new DoesNotHaveFoo());
}

<?hh

abstract class BaseCls {
  public function base() :mixed{}
}

class HasFoo extends BaseCls {
  public function foo0() :mixed{}
  public function foo1() :mixed{}
}

class HasFooChild1 extends HasFoo {}

class HasFooChild2 extends HasFoo {}

class DoesNotHaveFoo extends BaseCls {
  public function bar() :mixed{}
}

function fn(BaseCls $x) :mixed{
  $x->foo1();
}

<<__EntryPoint>>
function main() :mixed{
  fn(new HasFooChild1());
  fn(new HasFooChild2());
  fn(new DoesNotHaveFoo());
}

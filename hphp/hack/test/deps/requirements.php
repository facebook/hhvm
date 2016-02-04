<?hh

trait Bar {
  require extends FooParent<G>;
}

interface IFoo {
  require extends FooParent<G>;
}

class FooParent<T> {}
class G {}

class Foo extends FooParent<G> implements IFoo {
  use Bar;
}

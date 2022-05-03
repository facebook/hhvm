<?hh

interface IHasFoo {
  public function foo(): void;
}

interface IHasNothing {}

trait THasFoo implements IHasFoo {
}

trait THasBar {}

abstract class AbstractClass {}

// Bar is missing the `foo` method, but it's not clear how we ended up
// with this dependency without showing the route.
class Bar extends AbstractClass implements IHasNothing {
  use THasFoo; // It's actually this trait that caused us to need the
               // `foo` method.
  use THasBar;
}

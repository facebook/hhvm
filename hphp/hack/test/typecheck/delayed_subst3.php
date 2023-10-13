<?hh

abstract class Base<T> {
  public function test(T $data): void {}
}

trait MyTrait<T> {
  require extends Base<T>;
}

final class Child<T> extends Base<T> {
  use MyTrait<T>;
}

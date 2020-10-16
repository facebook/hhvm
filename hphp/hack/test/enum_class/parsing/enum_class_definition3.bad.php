<?hh
<<file:__EnableUnstableFeatures('enum_class')>>

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

trait MyTrait {
}

enum class E: ExBox {
  // Cannot use a trait inside an enum class
  use MyTrait;
  A<Box<string>>(new Box('bli'));
}

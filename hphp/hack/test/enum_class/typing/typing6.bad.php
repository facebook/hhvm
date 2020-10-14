<?hh
<<file: __EnableUnstableFeatures('enum_class')>>

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

enum class E : ExBox {
  // wrong initializer
  A<Box<string>>(new Box(42));
}

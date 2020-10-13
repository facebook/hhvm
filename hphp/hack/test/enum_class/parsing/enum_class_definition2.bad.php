<?hh
<<file:__EnableUnstableFeatures('enum_class')>>

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

enum class E: ExBox {
  A;
}

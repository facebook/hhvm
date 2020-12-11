<?hh
<<file:__EnableUnstableFeatures('enum_atom', 'enum_class')>>

enum class EE : I {
  A<Box>(new Box(42));
}

enum class FF : I extends EE {
  C<Box>(new Box(0));
}

enum class Foo: mixed {
  Str<Box<string>>(new Box('zuck'));
}

function ff(<<__Atom>> HH\Elt<EE, Box> $x) : int {
  return $x->unwrap()->x;
}

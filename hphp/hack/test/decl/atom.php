<?hh
<<file:__EnableUnstableFeatures('enum_atom', 'enum_class')>>

/* The file is missing definitions of I and Box on purpose, to make
 * the decl output less noisy
 */

enum class EE : I {
  Box A = new Box(42);
}

enum class FF : I extends EE {
  Box C = new Box(0);
}

enum class Foo: mixed {
   Box<string> Str = new Box('zuck');
}

function ff(<<__Atom>> HH\MemberOf<EE, Box> $x) : int {
  return $x->x;
}

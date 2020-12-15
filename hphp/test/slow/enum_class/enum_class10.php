<?hh
<<file: __EnableUnstableFeatures('enum_atom', 'enum_class')>>

interface IBox {}
class Box<T> implements IBox {
  public function __construct(public T $data) {}
}
enum class E : IBox {
  A<Box<string>>(new Box("world"));
}
function f<T>(<<__Atom>> HH\EnumMember<E, Box<T>> $elt) : T {
  return $elt->data()->data;
}

<<__EntryPoint>>
 function main() {
    $x = "A";
    echo("Hello " . f($x) . "!\n");
}

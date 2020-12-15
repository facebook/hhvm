<?hh
<<file: __EnableUnstableFeatures('enum_atom', 'enum_class')>>

interface IBox {}
class Box<T> implements IBox {
  public function __construct(public T $data) {}
}
enum class E : IBox {
  A<Box<string>>(new Box("world"));
}

function f<reify X, T>(<<__Atom>> HH\EnumMember<X, Box<T>> $elt) : T {
    return $elt->data()->data;
}

<<__EntryPoint>>
 function main() {
    $x = "A";
    echo("Hello " . f<E, string>($x) . "!\n");
}

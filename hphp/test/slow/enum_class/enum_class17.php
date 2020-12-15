<?hh
<<file: __EnableUnstableFeatures('enum_atom', 'enum_class')>>

interface IBox {}
class Box<T> implements IBox {
  public function __construct(public T $data) {}
}
function f<reify X, T>(<<__Atom>> HH\EnumMember<X, Box<T>> $elt) : T {
    return $elt->data()->data;
}

<<__EntryPoint>>
 function main() {
    $x = "A";
    echo("Hello " . f<string, string>($x) . "!\n");
}

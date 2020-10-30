<?hh
<<file: __EnableUnstableFeatures('enum_class')>>

interface IBox {}
class Box<T> implements IBox {
  public function __construct(public T $data) {}
}
type E = string;
function f<T>(<<__Atom>> HH\Elt<E, Box<T>> $elt) : T {
    return $elt->unwrap()->data;
}

<<__EntryPoint>>
 function main() {
    $x = "A";
    echo("Hello " . f($x) . "!\n");
}

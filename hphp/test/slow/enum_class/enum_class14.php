<?hh
<<file: __EnableUnstableFeatures('enum_class')>>

interface IBox {}
class Box<T> implements IBox {
  public function __construct(public T $data) {}
}
enum class E : IBox {
  A<Box<string>>(new Box("world"));
}
function f<T>(<<__Atom>> E $elt) : T {
  return $elt->unwrap()->data;
}

<<__EntryPoint>>
 function main() {
    $x = "A";
    echo("Hello " . f($x) . "!\n");
}

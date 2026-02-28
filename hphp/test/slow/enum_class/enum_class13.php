<?hh

interface IBox {}
class Box<T> implements IBox {
  public function __construct(public T $data)[] {}
}
enum class E : IBox {
   Box<string> A = new Box("world");
}
function f<T>(HH\MemberOf<E, Box<T>> $elt) : T {
  return $elt->data;
}

<<__EntryPoint>>
 function main() :mixed{
    $x = "A";
    echo("Hello " . (string)(f($x)) . "!\n");
}

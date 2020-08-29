<?hh

class Foo {}
class Bar {
  const FOO = Foo::class;
}

<<__EntryPoint>>
function main() {
  $c = Bar::class;
  var_dump($c);
  $v = vec[Bar::class, Fizz::class]; // Fizz is not a class
  var_dump($v);
  var_dump(Bar::FOO);
}

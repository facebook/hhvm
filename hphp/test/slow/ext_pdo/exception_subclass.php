<?hh

class Foo extends PDOException {}

function main() :mixed{
  // just checking it can be default-constructed...
  var_dump((new Foo())->getMessage());

  $junk = new Exception();
  $foo = new Foo('hello, world', 1337, $junk);
  var_dump($foo->getMessage());
  var_dump($foo->getCode());
  var_dump($foo->getPrevious() === $junk);
}


<<__EntryPoint>>
function main_exception_subclass() :mixed{
main();
}

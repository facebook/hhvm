<?hh

class Foo {
  function __invoke(...$args) {
    yield __FUNCTION__;
    foreach ($args as $arg) {
      yield $arg;
    }
  }
}


<<__EntryPoint>>
function main_magic_methods_ok() {
$foo = new Foo();
var_dump(iterator_to_array($foo('d', 'e', 'f')));
}

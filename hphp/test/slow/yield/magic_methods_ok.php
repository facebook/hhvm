<?hh

class Foo {
  function __invoke(...$args) {
    yield __FUNCTION__;
    foreach ($args as $arg) {
      yield $arg;
    }
  }
  function __call($a, $b) {
    yield __FUNCTION__;
    yield $a;
    yield $b;
  }
}


<<__EntryPoint>>
function main_magic_methods_ok() {
$foo = new Foo();
var_dump(iterator_to_array($foo('d', 'e', 'f')));
var_dump(iterator_to_array($foo->bar('d', 'e', 'f')));
}

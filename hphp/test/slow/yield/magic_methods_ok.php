<?hh

class Foo {
  function __invoke(...$args) :AsyncGenerator<mixed,mixed,void>{
    yield __FUNCTION__;
    foreach ($args as $arg) {
      yield $arg;
    }
  }
}


<<__EntryPoint>>
function main_magic_methods_ok() :mixed{
$foo = new Foo();
var_dump(iterator_to_array($foo('d', 'e', 'f')));
}

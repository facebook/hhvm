<?hh

class Foo {
  public function bar(callable $baz) {}
}


<<__EntryPoint>>
function main_bug_3289_reflectionparameter_getclass_callable() {
var_dump((new ReflectionParameter(varray['Foo', 'bar'], 'baz'))->getClass());
}

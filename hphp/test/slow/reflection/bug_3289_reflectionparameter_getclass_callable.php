<?hh

class Foo {
  public function bar(callable $baz) :mixed{}
}


<<__EntryPoint>>
function main_bug_3289_reflectionparameter_getclass_callable() :mixed{
var_dump((new ReflectionParameter(vec['Foo', 'bar'], 'baz'))->getClass());
}

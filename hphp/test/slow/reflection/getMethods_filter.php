<?hh

abstract class Test {
  abstract public function myMethod():mixed;
}


<<__EntryPoint>>
function main_get_methods_filter() :mixed{
$refl = new ReflectionClass('Test');
var_dump($refl->getMethods(ReflectionMethod::IS_ABSTRACT)[0]->name);
}

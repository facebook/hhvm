<?hh

abstract class Test {
  abstract public function myMethod();
}


<<__EntryPoint>>
function main_get_methods_filter() {
$refl = new ReflectionClass('test');
var_dump($refl->getMethods(ReflectionMethod::IS_ABSTRACT)[0]->name);
}

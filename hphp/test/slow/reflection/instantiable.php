<?hh

class A {}
trait B {}
interface C {}
abstract class D {}
class E {
  protected function __construct()[] {}
}

<<__EntryPoint>>
function main_instantiable() :mixed{
var_dump((new ReflectionClass('A'))->isInstantiable());
var_dump((new ReflectionClass('B'))->isInstantiable());
var_dump((new ReflectionClass('C'))->isInstantiable());
var_dump((new ReflectionClass('D'))->isInstantiable());
var_dump((new ReflectionClass('E'))->isInstantiable());
}

<?hh

namespace Test;

interface TestInterface {

}

class TestClass implements TestInterface {

}


<<__EntryPoint>>
function main_reflection_class_implements_interface() :mixed{
$reflection = new \ReflectionClass('\Test\TestClass');

\var_dump($reflection->implementsInterface('\Test\TestInterface'));
}

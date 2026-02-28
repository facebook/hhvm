<?hh

class a {}
abstract class b {}
final class c {}

class x
{
    function __construct() {}
    private function a() :mixed{}
    private static function b() :mixed{}
    protected function c() :mixed{}
    protected static function d() :mixed{}
    public function e() :mixed{}
    public static function f() :mixed{}
    final function g() :mixed{}
    function h() :mixed{}
}

abstract class y
{
    abstract function a():mixed;
    abstract protected function b():mixed;
}

function dump_modifierNames($class) :mixed{
    $obj = new ReflectionClass($class);
    var_dump($obj->getName(), Reflection::getModifierNames($obj->getModifiers()));
}

function dump_methodModifierNames($class) :mixed{
    $obj = new ReflectionClass($class);
    foreach($obj->getMethods() as $method) {
        var_dump($obj->getName() . "::" . $method->getName(), Reflection::getModifierNames($method->getModifiers()));
    }
}
<<__EntryPoint>> function main(): void {
dump_modifierNames('a');
dump_modifierNames('b');
dump_modifierNames('c');

dump_methodModifierNames('x');
dump_methodModifierNames('y');
echo "==DONE==";
}

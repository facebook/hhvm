<?hh
class C {
}

interface iface {
    function f1():mixed;
}

class ifaceImpl implements iface {
    function f1() :mixed{}
}

abstract class abstractClass {
    function f1() :mixed{}
    abstract function f2():mixed;
}

class D extends abstractClass {
    function f2() :mixed{}
}
<<__EntryPoint>> function main(): void {
$classes = vec["C", "iface", "ifaceImpl", "abstractClass", "D"];

foreach($classes  as $class ) {
    $reflectionClass = new ReflectionClass($class);
    echo "Is $class instantiable?  ";
    var_dump($reflectionClass->isInstantiable());

}
}

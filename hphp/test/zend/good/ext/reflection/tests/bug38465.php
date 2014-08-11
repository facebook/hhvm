<?php
class Baz {
    const B = 3;
}

class Foo {
    const X = 1;
    public function x($a = self::X, $b = Baz::B, $c = 99) {}
}

class Bar extends Foo {
    const Y = 2;
    public function y($a = self::Y, $b = Baz::B, $c = 99) {}
}


echo "From global scope:\n";

$clazz = new ReflectionClass('Bar');
foreach ($clazz->getMethods() as $method) {
    foreach ($method->getParameters() as $param) {
        if ($param->isDefaultValueAvailable()) {
            echo $method->getDeclaringClass()->getName(), '::', $method->getName(), '($', $param->getName(), ' = ', $param->getDefaultValue(), ")\n";
        }
    }
}

echo "\nFrom class context:\n";

class Test {
    function __construct() {
        $clazz = new ReflectionClass('Bar');
        foreach ($clazz->getMethods() as $method) {
            foreach ($method->getParameters() as $param) {
                if ($param->isDefaultValueAvailable()) {
                    echo $method->getDeclaringClass()->getName(), '::', $method->getName(), '($', $param->getName(), ' = ', $param->getDefaultValue(), ")\n";
                }
            }
        }
    }
}

new Test();

?>

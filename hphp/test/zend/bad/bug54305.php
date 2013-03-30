<?php
class TestClass {
    public function methodWithArgs($a, $b) {
    }
}
abstract class AbstractClass {
}
$methodWithArgs = new ReflectionMethod('TestClass', 'methodWithArgs');
echo $methodWithArgs++;
?>
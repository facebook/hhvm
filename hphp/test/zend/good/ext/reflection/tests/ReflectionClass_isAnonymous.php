<?php
class TestClass {}
$declaredClass = new ReflectionClass('TestClass');
$anonymousClass = new ReflectionClass(new class {});
var_dump($declaredClass->isAnonymous());
var_dump($anonymousClass->isAnonymous());
?>

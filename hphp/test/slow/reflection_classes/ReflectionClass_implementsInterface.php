<?php

namespace Test;

interface TestInterface {

}

class TestClass implements TestInterface {

}

$reflection = new \ReflectionClass('\Test\TestClass');

var_dump($reflection->implementsInterface('\Test\TestInterface'));

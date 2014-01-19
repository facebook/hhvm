<?php

namespace Main\Test;

class MyClass {
}

$i = new MyClass;
$badName = '\Main\Test\MyClass';
$goodName = 'Main\Test\MyClass';

function testClass($i, $className) {
    printf("%s\n", class_exists($className) ? 'YES' : 'NO');
    printf("%s\n", ($i instanceof $className) ? 'YES' : 'NO');
}

testClass($i, $badName);
testClass($i, $goodName);

<?php

class TestClass {
}

$a = 5;

echo "Non-existent class:\n";
try {
    $propInfo = new ReflectionProperty("NonExistentClass", "prop");
}
catch(Exception $e) {
    echo $e->getMessage();
}

echo "\n\nWrong property parameter type:\n";
try {
    $propInfo = new ReflectionProperty($a, 'TestClass');
}
catch(ReflectionException $e) {
    echo $e->getMessage();
}

echo "\n\nNon-existent property:\n";
try {
    $propInfo = new ReflectionProperty('TestClass', "nonExistentProperty");
}
catch(Exception $e) {
    echo $e->getMessage();
}

?>

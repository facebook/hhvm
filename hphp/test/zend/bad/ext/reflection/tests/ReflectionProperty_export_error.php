<?php

class TestClass {
}

$a = 5;

echo "Non-existent class:\n";
try {
    ReflectionProperty::export("NonExistentClass", "prop", true);
}
catch(Exception $e) {
    echo $e->getMessage();
}

echo "\n\nWrong property parameter type:\n";
try {
    ReflectionProperty::export($a, 'TestClass', false);
}
catch(ReflectionException $e) {
    echo $e->getMessage();
}

echo "\n\nNon-existent property:\n";
try {
    ReflectionProperty::export('TestClass', "nonExistentProperty", true);
}
catch(Exception $e) {
    echo $e->getMessage();
}

echo "\n\nIncorrect number of args:\n";
ReflectionProperty::export();
ReflectionProperty::export('TestClass', "nonExistentProperty", true, false);

?>

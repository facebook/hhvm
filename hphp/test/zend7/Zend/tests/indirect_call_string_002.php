<?php
class TestClass
{
    public static function __callStatic($method, array $args)
    {
        var_dump($method);
    }
}

// Test call using array syntax
$callback = ['TestClass', ''];
$callback();

// Test call using Class::method syntax.
$callback = 'TestClass::';
$callback();

// Test array syntax with empty class name
$callback = ['', 'method'];
try {
    $callback();
} catch (Error $e) {
    echo $e->getMessage() . "\n";
}

// Test Class::method syntax with empty class name
$callback = '::method';
try {
    $callback();
} catch (Error $e) {
    echo $e->getMessage() . "\n";
}

// Test array syntax with empty class and method name
$callback = ['', ''];
try {
    $callback();
} catch (Error $e) {
    echo $e->getMessage() . "\n";
}

// Test Class::method syntax with empty class and method name
$callback = '::';
try {
    $callback();
} catch (Error $e) {
    echo $e->getMessage() . "\n";
}

// Test string ending in single colon
$callback = 'Class:';
try {
    $callback();
} catch (Error $e) {
    echo $e->getMessage() . "\n";
}

// Test string beginning in single colon
$callback = ':method';
try {
    $callback();
} catch (Error $e) {
    echo $e->getMessage() . "\n";
}

// Test single colon
$callback = ':';
try {
    $callback();
} catch (Error $e) {
    echo $e->getMessage() . "\n";
}
?>

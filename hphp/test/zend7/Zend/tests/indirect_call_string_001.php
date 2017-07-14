<?php
namespace TestNamespace
{
    class TestClass
    {
        public static function staticMethod()
        {
            echo "Static method called!\n";
        }
        
        public static function staticMethodWithArgs($arg1, $arg2, $arg3)
        {
            printf("Static method called with args: %s, %s, %s\n", $arg1, $arg2, $arg3);
        }
    }
}

namespace CallNamespace
{
    // Test basic call using Class::method syntax.
    $callback = 'TestNamespace\TestClass::staticMethod';
    $callback();

    // Case should not matter.
    $callback = 'testnamespace\testclass::staticmethod';
    $callback();

    $args = ['arg1', 'arg2', 'arg3'];
    $callback = 'TestNamespace\TestClass::staticMethodWithArgs';

    // Test call with args.
    $callback($args[0], $args[1], $args[2]);

    // Test call with splat operator.
    $callback(...$args);
}
?>

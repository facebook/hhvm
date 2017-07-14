<?php
class Foo
{
    // nothing
}

function FooTest(Foo $foo)
{
    echo "Hello!";
}

function NormalTest($a)
{
    echo "Hi!";
}

try {
	NormalTest();
} catch (Throwable $e) {
	echo "Exception: " . $e->getMessage() . "\n";
}
try {
	FooTest();
} catch (Throwable $e) {
	echo "Exception: " . $e->getMessage() . "\n";
}
FooTest(new Foo());
?>

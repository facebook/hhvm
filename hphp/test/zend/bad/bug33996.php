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

NormalTest();
FooTest();
FooTest(new Foo());
?>
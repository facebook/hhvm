<?php

class TestClass
{
    function __construct()
    {
        self::Test1();
    }

    static function Test1()
    {
        var_dump($this);
    }

    static function Test2($this)
    {
        var_dump($this);
    }
}
<<__EntryPoint>> function main() {
$obj = new TestClass;
TestClass::Test2(new stdClass);

echo "===DONE===\n";
}

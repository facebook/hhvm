<?php

class ClassName
{
    public $var = 'bla';
}

function test (OtherClassName $object) { }

function __autoload($class)
{
    var_dump("__autload($class)");
}
<<__EntryPoint>> function main() {
$obj = new ClassName;
test($obj);

echo "Done\n";
}

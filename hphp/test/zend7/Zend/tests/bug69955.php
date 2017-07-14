<?php
class C10 implements ArrayAccess
{
        function offsetExists($offset)
        {
                echo "\nInside " . __METHOD__ . "\n"; var_dump($offset);
        }
        function offsetGet($offset)
        {
                echo "\nInside " . __METHOD__ . "\n"; var_dump($offset); return 100;
        }
        function offsetSet($offset, $value)
        {
                echo "\nInside " . __METHOD__ . "\n"; var_dump($offset); var_dump($value);
        }
        function offsetUnset($offset)
        {
                echo "\nInside " . __METHOD__ . "\n"; var_dump($offset);
        }
}

$c10 = new C10;

var_dump($c10[] += 5);

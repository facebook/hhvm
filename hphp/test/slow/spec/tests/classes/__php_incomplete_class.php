<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class Point
{
    private $x;
    private $y;

    public function __construct($x = 0, $y = 0)
    {
        $this->x = $x;
        $this->y = $y;

        echo "\nInside " . __METHOD__ . ", $this\n\n";
    }

    public function __toString()
    {
        return '(' . $this->x . ',' . $this->y . ')';
    }   
}

echo "---------------- create, serialize, and unserialize a Point -------------------\n";

$p = new Point(2, 5);
echo "Point \$p = $p\n";

$s = serialize($p);     // all instance properties get serialized
var_dump($s);

echo "------\n";

$v = unserialize($s);   // without a __wakeup method, any instance property present
                        // in the string takes on its default value.
var_dump($v);

$s[5] = 'J';        // change class name, so a unserialize failure occurs
var_dump($s);
$v = unserialize($s);
var_dump($v);
print_r($v);

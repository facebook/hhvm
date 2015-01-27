<?php

error_reporting(-1);

class Point
{
    private $x;
    private $y;
    private $dynamicProperties = array();

    public $dummy = -100;   // for test purposes only

    public function __construct($x = 0, $y = 0)
    {
        $this->x = $x;
        $this->y = $y;
    }

    public function __set($name, $value)
    {
//          echo __METHOD__ . "($name, $value)\n";
        echo __METHOD__ . "($name, xx)\n"; // used if $value can't be converted to string

        $this->dynamicProperties[$name] = $value;
    }

    public function __get($name)
    {
        echo __METHOD__ . "($name)\n";

        if (array_key_exists($name, $this->dynamicProperties))
        {
            return $this->dynamicProperties[$name];
        }

        // no-such-property error handling goes here
        return null;
    }

    public function __isset($name)
    {
        echo __METHOD__ . "($name)\n";

        return isset($this->dynamicProperties[$name]);
    }

    public function __unset($name)
    {
        echo __METHOD__ . "($name)\n";

        unset($this->dynamicProperties[$name]);
    }
}

$p = new Point(5, 9);

echo "----------------------\n";

$v = $p->dummy;             // get visible property
echo "dummy: $v\n";
$v = $p->__get('dummy');    // get dynamic property, if one exists; else, fails
echo "dynamic dummy: $v\n";

$p->dummy = 987;            // set visible property
$p->__set('dummy', 456);    // set dynamic property

$v = $p->dummy;             // get visible property
echo "dummy: $v\n";
$v = $p->__get('dummy');    // get dynamic property
echo "dynamic dummy: $v\n";

echo "----------------------\n";

var_dump(isset($p->dummy));     // test if dummy exists and is accessible, or is dynamic
var_dump($p->__isset('dummy')); // test if dynamic dummy exists

echo "----------------------\n";

$v = $p->x;     // try to get at an invisible property; can't. The runtime sees that x
                // exists, but is invisible, so it calls __get to search for a dynamic
                // property of that name, which fails. NULL is returned.
var_dump($v);

echo "----------------------\n";

var_dump(isset($p->x));     // test if x exists and is accessible, or is dynamic
var_dump($p->__isset('x')); // test if x exists and is accessible, or is dynamic

$p->x = 200;
var_dump($p->x);

var_dump(isset($p->x));     // test if x exists and is accessible, or is dynamic
var_dump($p->__isset('x')); // test if x exists and is accessible, or is dynamic

echo "----------------------\n";

$p->color = "red";          // set dynamic property
$v = $p->color;             // get dynamic property
echo "color: $v\n";

echo "----------------------\n";

var_dump(isset($p->color)); // test if color exists and is accessible, or is dynamic

echo "----------------------\n";

$v = $p->dummy = 555;
echo "\$v: $v, dummy: " . $p->dummy . "\n";

$v = $p->color = "White";       // this calls __set but not __get
echo "\$v: $v, color: " . $p->color . "\n";

echo "----------------------\n";

var_dump(isset($p->dummy));
var_dump($p->__isset('dummy')); // test if x exists and is accessible, or is dynamic
$p->__unset('dummy');
var_dump(isset($p->dummy));
var_dump($p->__isset('dummy')); // test if x exists and is accessible, or is dynamic

unset($p->abc);             // request to unset a non-existent is ignored
unset($p->x);               // request to unset an inaccessible is ignored
var_dump(isset($p->dummy));
unset($p->dummy);           // request to unset a declared accessible is OK
var_dump(isset($p->dummy));

var_dump(isset($p->color));
unset($p->color);           //
var_dump(isset($p->color));

echo "----------------------\n";

class X
{
    public function __destruct()
    {
        echo __METHOD__ . "\n";
    }
}

///*
$p->thing = new X;  // set dynamic property to an instance having a destructor
$v = $p->thing;
var_dump($v);

//unset($p->thing);   // was sort-of expecting this to trigger the destructor, but ...
//$p->__unset('thing');
//echo "unset(\$p->thing) called\n";
//*/

// at program termination, the destructor for the dynamic property is called

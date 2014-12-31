<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

echo "--------- TRUE -------------\n";

$v = TRUE;
var_dump(isset($v));
unset($v);
var_dump(isset($v));

echo "--------- NULL -------------\n";

$v = NULL;
var_dump(isset($v));
unset($v);
var_dump(isset($v));

echo "--------- TRUE, 12.3, NULL -------------\n";

$v1 = TRUE; $v2 = 12.3; $v3 = "abc";
var_dump(isset($v1, $v2, $v3));
unset($v1, $v2, $v3);
var_dump(isset($v1, $v2, $v3));

echo "--------- undefined parameter -------------\n";

function f($p)
{
    var_dump($p);
    var_dump(isset($p));
    unset($p);
    var_dump(isset($p));
}

f();
f(NULL);
f(10);

echo "---------- dynamic property ------------\n";

class X1
{
}

class X2
{
    public function __isset($name)
    {
        echo "Inside " . __METHOD__ . " with \$name $name\n";
    }

    public function __unset($name)
    {
        echo "Inside " . __METHOD__ . " with \$name $name\n";
    }
}

$x1 = new X1;
var_dump(isset($x1->m));
$x1->m = 123;
var_dump(isset($x1->m));
unset($x1->m);
var_dump(isset($x1->m));

$x2 = new X2;
var_dump(isset($x2->m));
unset($x2->m);
var_dump(isset($x2->m));

echo "---------- unsetting inside a function (global) ------------\n";

$gl = 100;

function g1()
{
    global $gl;
    var_dump(isset($gl));
    unset($gl);             // unsets local "version" in current scope
    var_dump(isset($gl));
}

g1();
var_dump(isset($gl));       // still set

echo "---------- unsetting inside a function (\$GLOBALS) ------------\n";

function g2()
{
    var_dump(isset($GLOBALS['gl']));
    unset($GLOBALS['gl']);              // unsets global "version"
    var_dump(isset($GLOBALS['gl']));
}

g2();
var_dump(isset($gl));       // no longer set

echo "---------- unsetting inside a function (pass-by-ref) ------------\n";

function g3($p1, &$p2)
{
    var_dump(isset($p1, $p2));
    unset($p1, $p2);            // unsets local "version" in current scope
    var_dump(isset($p1, $p2));
}

$v1 = 10;
$v2 = 20;
g3($v1, $v2);
var_dump(isset($v1));       // still set
var_dump($v1);
var_dump(isset($v2));       // still set, even though passed in by reference and unset
var_dump($v2);

echo "---------- unsetting inside a function (static) ------------\n";

function g4()
{
    static $count = 0;
    ++$count;
    echo "count = $count\n";

    var_dump(isset($count));
    unset($count);          // unsets local "version" in current scope
    var_dump(isset($count));
}

g4();
g4();

echo "---------- unsetting a property ------------\n";

class C
{
    const CON1 = 123;
    public $prop = 10;
    public static $sprop = -5;
}

$c1 = new C;
var_dump($c1);
var_dump(isset($c1->prop));
unset($c1->prop);           // remove it from this instance
var_dump(isset($c1->prop));

//unset(C::$sprop);         // Attempt to unset static property

var_dump($c1);

echo "-----------\n";

$c2 = new C;
var_dump($c2);
var_dump(isset($c2->prop));

echo "---------- unsetting \$this ------------\n";

class D
{
    public function f()
    {
        echo "Inside ". __METHOD__ . "\n";

        var_dump(isset($this));
        unset($this);
        var_dump(isset($this));

        $this->g();     // use $this to call sibling instance method
    }

    private function g()
    {
        echo "Inside ". __METHOD__ . "\n";
        
        var_dump(isset($this));
    }
}

$d = new D;
$d->f();

echo "---------- unsetting array elements ------------\n";

$a = array(10, 20, "xx" => 30);
print_r($a);
unset($a[1]);
print_r($a);

unset($a[10]);
print_r($a);

unset($a["Xx"]);
print_r($a);

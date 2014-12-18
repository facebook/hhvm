<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

echo "----------------- closure with no parameters ----------------------\n";

$cl1 = function ()
{
    echo "Inside function >>" . __FUNCTION__ . "<<\n";
    echo "Inside method >>" . __METHOD__ . "<<\n";
    // ...
};

echo "--\n";
var_dump(gettype($cl1));
echo "--\n";
var_dump($cl1);
echo "--\n";
var_dump($cl1 instanceof Closure);
echo "--\n";

$cl1();

// Closure object is empty

echo "----------------- closure with 4 parameters ----------------------\n";

interface I {}
class C implements I {}

$cl2 = function ($p1, $p2 = 100, array $p3, C $p4, I $p5)
{
    echo "Inside function >>" . __FUNCTION__ . "<<\n";
    echo "Inside method >>" . __METHOD__ . "<<\n";
    // ...
};
var_dump($cl2);

echo "--\n";
var_dump(gettype($cl2));
echo "--\n";
var_dump($cl2);
echo "--\n";
var_dump($cl2 instanceof Closure);
echo "--\n";

$cl2(10, 20, [1,2], new C, new C);

echo "----------------- passing a callable to a function ----------------------\n";

function double($p)
{
    return $p * 2;
}

function square($p)
{
    return $p * $p;
}

function doit($value, callable $process)
{
    var_dump($process);

    return $process($value);
}

$res = doit(10, 'double');
echo "Result of calling doit using function double = $res\n-------\n";

$res = doit(10, 'square');
echo "Result of calling doit using function square = $res\n-------\n";


$res = doit(5, function ($p) { return $p * 2; });
echo "Result of calling doit using double closure = $res\n-------\n";

$res = doit(5, function ($p) { return $p * $p; });
echo "Result of calling doit using square closure = $res\n-------\n";

echo "----------------- using a use clause, #1 ----------------------\n";

function compute(array $values)
{
    $count = 0;

    $callback = function () use (&$count)
    {
        echo "Inside method >>" . __METHOD__ . "<<\n";  // called {closure}
        ++$count;
    };

    $callback();
    echo "\$count = $count\n";
    $callback();
    echo "\$count = $count\n";
}

compute([1,2,3]);

echo "----------------- using a use clause, #2 (instance method) ----------------------\n";

class D
{
    private function f()
    {
        echo "Inside method >>" . __METHOD__ . "<<\n";
    }

    public function compute(array $values)
    {
        $count = 0;

        $callback = function ($p1, $p2) use (&$count, $values)
        {
            echo "Inside method >>" . __METHOD__ . "<<\n";  // called D::{closure}
            ++$count;

            $this->f(); // $this is available automatically; can't put it in use clause anyway
        };

        echo "--\n";
        var_dump(gettype($callback));
        echo "--\n";
        var_dump($callback);
        echo "--\n";
        var_dump($callback instanceof Closure);
        echo "--\n";

        $callback(1,2,3);
        echo "\$count = $count\n";
        $callback(5,6,7);
        echo "\$count = $count\n";

        $callback2 = function()
        {
            echo "Inside method >>" . __METHOD__ . "<<\n";  // ALSO called D::{closure}
        };

        echo "--\n";
        var_dump(gettype($callback2));
        echo "--\n";
        var_dump($callback2);
        echo "--\n";
        var_dump($callback2 instanceof Closure);
        echo "--\n";

        $callback2();
    }

    public static function stcompute(array $values)
    {
        $count = 0;

        $callback = function ($p1, $p2) use (&$count, $values)
        {
            echo "Inside method >>" . __METHOD__ . "<<\n";  // called D::{closure}
            ++$count;
        };

        echo "--\n";
        var_dump(gettype($callback));
        echo "--\n";
        var_dump($callback);
        echo "--\n";
        var_dump($callback instanceof Closure);
        echo "--\n";

        $callback(1,2,3);
        echo "\$count = $count\n";
        $callback(5,6,7);
        echo "\$count = $count\n";
    }

}

$d1 = new D;
$d1->compute(["red" => 3, 10]);

echo "----------------- using a use clause, #3 (static method) ----------------------\n";

D::stcompute(["red" => 3, 10]);

echo "----------------- Misc. Stuff ----------------------\n";

//(function () { echo "Hi\n"; })();     // ca't use an anon function directly with ()
$v = (function () { echo "Hi\n"; });
$v();

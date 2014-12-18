<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

echo "--------------- test type hint array ---------------------\n";

function f1(array $p1)
{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
}

// f1();    // Argument 1 passed to f1() must be of the type array, none given
// f1(123); // Argument 1 passed to f1() must be of the type array, integer given
f1([10,20]);

echo "--------------- test type hint class-name ---------------------\n";

class C1 {}
class D1 extends C1 {}

function f2(C1 $p1)
{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
}

//f2(123); // Argument 1 passed to f1() must be an instance of C1, integer give
//f2([10,20]);    // Argument 1 passed to f2() must be an instance of C1, array given
f2(new C1);
f2(new D1);

echo "--------------- test type hint object ---------------------\n";

function f3(object $p1)
{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
}

//f3(123); // Argument 1 passed to f1() must be an instance of object, integer given
//f3([10,20]);    // Argument 1 passed to f2() must be an instance of object, array given
//f3(new C1);         // must be an instance of object, instance of C1 given

// object is not a special/recognized marker in this context

echo "--------------- test type hint interface-name ---------------------\n";

interface I1 {}
interface I2 extends I1 {}
class C2 implements I1 {}
class D2 extends C2 implements I2 {}

function f4(I1 $p1)
{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
}

//f4(123); // must implement interface I1, integer given
f4(new C2);
f4(new D2);

echo "--------------- test type hint callable ---------------------\n";

function hello()
{
    echo "Hello!\n";
}

function f5(callable $p1)
{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
    $p1();
}

//f5(123); // must be callable, integer given
f5('hello');

echo "--------------- test type hint + by ref ---------------------\n";

function f6(C1 &$p1)
{
    echo "Inside " . __METHOD__ . "\n";

    var_dump($p1);
}

f6(new C1);

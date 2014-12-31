<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

echo "Inside >" . __TRAIT__ . "<\n";
echo "Inside >" . __CLASS__ . "<\n";
echo "Inside >" . __METHOD__ . "<\n";
echo "Inside >" . __FUNCTION__ . "<\n";

echo "===================== Test an Empty Trait =========================\n";

trait T1 {}     // allowed to be empty

class C1 { use T1; }

echo "========== Test Overriding and Collisions Between Traits =====\n";

trait T2a
{
    function f()
    {
        echo "Inside " . __TRAIT__ . "\n";
        echo "Inside " . __CLASS__ . "\n";
        echo "Inside " . __METHOD__ . "\n";
    }
}

trait T2b
{
//  function f($p1, $p2) // signatures not factored in when looking for name clashes
    function f()
    {
        echo "Inside " . __TRAIT__ . "\n";
        echo "Inside " . __CLASS__ . "\n";
        echo "Inside " . __METHOD__ . "\n";
    }
}

class C2Base
{
    public function f() { echo "Inside " . __METHOD__ . "\n"; }
}

class C2Derived extends C2Base
{
//  use T2a; use T2b;   // equivalent to use T2a, T2b;
//  use T2a, T2b;       // clash between two names f REGARDLESS of argument lists
    use T2a, T2b
    {   // with both below excepted, went to base, bypassing both traits!!
        T2a::f insteadof T2b;
//      T2b::f insteadof T2a;

        T2b::f as g;    // allow otherwise hidden T2B::f to be seen through alias g
        T2a::f as h;    // allow T2a::f to also be seen through alias h
                        // don't need qualifier prefix if f is unambiguous
    }

//  public function f() { echo "Inside " . __METHOD__ . "\n"; }
}

$c2 = new C2Derived;

echo "-------\n";
$c2->f();       // call T2a::f

echo "-------\n";
$c2->g();       // call T2b::f via its alias g

echo "-------\n";
$c2->h();       // call T2a::f via its alias h

// confirmed that lookup starts with current class, then trait(s), then base classes

echo "===================== Changing Visibility =========================\n";

trait T3
{
    public function m1() { echo "Inside " . __METHOD__ . "\n"; }
    protected function m2() { echo "Inside " . __METHOD__ . "\n"; }
    private function m3() { echo "Inside " . __METHOD__ . "\n"; }

    function m4() { echo "Inside " . __METHOD__ . "\n"; }   // implicitly public
}

class C3
{
    use T3
    {
        m1 as protected;        // reduce visibility to future, derived classes
        m2 as private;
        m3 as public;
        m3 as protected z3;
    }
}

$c3 = new C3;
//$c3->m1();        // accessible, by default, but not once protected
//$c3->m2();        // inaccessible, by default
$c3->m3();          // inaccessible, by default
$c3->m4();          // accessible, by default

echo "===================== Traits using other Traits =========================\n";


trait Tx1
{
    function k()
    {
        echo "Inside " . __TRAIT__ . "\n";
        echo "Inside " . __CLASS__ . "\n";
        echo "Inside " . __METHOD__ . "\n";
    }
}

trait Tx2
{
    function m()
    {
        echo "Inside " . __TRAIT__ . "\n";
        echo "Inside " . __CLASS__ . "\n";
        echo "Inside " . __METHOD__ . "\n";
    }
}

trait T4
{
    use Tx1, Tx2;
    use T2a, T2b, T3
    {
        Tx1::k as kk;
        T2a::f insteadof T2b;
    }
}

class C4
{
    use T4;
}

$c4 = new C4;

echo "-------\n";
$c4->f();

echo "-------\n";
$c4->m1();

echo "-------\n";
$c4->k();

echo "-------\n";
$c4->m();

echo "===================== static properties =========================\n";

trait T5
{
    public static $prop;
}

class C5a
{
    use T5;
}

class C5b
{
    use T5;
}

C5a::$prop = 123;
C5b::$prop = "red";
echo C5a::$prop . "\n"; // ==> 123
echo C5b::$prop . "\n"; // ==> red

echo "===================== function statics =========================\n";

trait T6
{
    public function f()
    {
        echo "Inside " . __METHOD__ . "\n";

        static $v = 0;          // static is class-specific
        echo "\$v = " . $v++ . "\n";
    }
}

class C6a
{
    use T6;
}

class C6b
{
    use T6;
}

$v1 = new C6a;
$v1->f();       // method run twice with same $v
$v1->f();

echo "-------\n";

$v2 = new C6b;
$v2->f();       // method run three times with a different $v
$v2->f();
$v2->f();

echo "===================== Using a Trait without a Class =========================\n";

trait T7
{
    public static $pubs = 123;

    function f()    // implicitly public
    {
        echo "Inside " . __TRAIT__ . "\n";
        echo "Inside " . __CLASS__ . "\n";
        echo "Inside " . __METHOD__ . "\n";
        var_dump($this);
    }

    public static function g()
    {
        echo "Inside " . __TRAIT__ . "\n";
        echo "Inside " . __CLASS__ . "\n";
        echo "Inside " . __METHOD__ . "\n";
    }
}

T7::f();    // calls f like a static function with class name being the trait name

echo "-------\n";
T7::g();

/*
echo "-------\n";
var_dump(T7::pubs); // doesn't work for static properties
*/

echo "===================== examples for spec =========================\n";

trait T9a
{
    public function compute(/* ... */) { /* ... */ }
}

trait T9b
{
    public function compute(/* ... */) { /* ... */ }
}

trait T9c
{
    public function sort(/* ... */) { /* ... */ }
}

trait T9d
{
    use T9c;
    use T9a, T9b
    {
        T9a::compute insteadof T9b;
        T9c::sort as private sorter;
    }
}

trait T10
{
    private $prop1 = 1000;
    protected static $prop2;
    var $prop3;
    public function compute() {}
    public static function getData() {}
}

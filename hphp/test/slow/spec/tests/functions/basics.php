<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

// Function names are not case-sensitive

//function f() { echo "f\n"; }
//function F() { echo "F\n"; }  // F is equivalent to f

// function having no declared parameters

function f1()
{
    $argList = func_get_args();
    echo "f1: # arguments passed is ".count($argList)."\n";

    foreach ($argList as $k => $e)
    {
        echo "\targ[$k] = >$e<\n";
    }
}

var_dump(f1()); // call f1, default return value is NULL
f1;             // valid, but vacuous, as it has no side effect and its value is not used
var_dump(f1);   // string with value "f1"
$f = f1;        // assign this string to a variable
$f();           // call f1 indirectly via $f
//"f1"();           // call f1 via the string "f1" -- Can't be a string literal!!!

// f1() = 123;  // a function return is not an lvalue

f1();
f1(10);
f1(TRUE, "green");
f1(23.45, NULL, array(1,2,3));

// function having 2 declared parameters

function f2($p1, $p2)
{
    // A NULL value doesn't prove the argument wasn't passed; find a better test

    echo "f2: \$p1 = ".($p1 == NULL ? "NULL" : $p1).
        ", \$p2 = ".($p2 == NULL ? "NULL" : $p2)."\n";
}

// if fewer arguments are passed than there are paramaters declared, a warning is issued
// and the parameters corresponding to each each omitted argument are undefined

f2();           // pass 0 (< 2)
f2(10);         // pass 1 (< 2)
f2(10, 20);     // pass 2 (== 2)
f2(10, 20, 30); // pass 3 (> 2)

// some simple examples of function calls

function square($v) { return $v * $v; }
echo "5 squared = ".square(5)."\n";
var_dump($funct = square);
var_dump($funct(-2.3));

echo strlen("abcedfg")."\n";

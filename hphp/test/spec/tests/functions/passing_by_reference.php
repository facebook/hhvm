<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

// A useful pass-by-reference example; swap the values of two variables

function swap(&$p1, &$p2)
{
   $temp = $p1;
   $p1 = $p2;
   $p2 = $temp;
}

$a1 = 1.23e27;
$a2 = [10,TRUE,NULL];
var_dump($a1);
var_dump($a2);
swap($a1, $a2);
var_dump($a1);
var_dump($a2);

///*
// a simple example of passing by reference

function f(&$p)
{
   echo '$p '.(isset($p) ? "is set\n" : "is not set\n");
    echo "f In:  \$p: $p\n";
    $p = 200;       // actual argument's value changed
    echo "f Out: \$p: $p\n";
}

// pass a variable by reference; f changes its value

$a = 10;
var_dump($a);
f($a);   // change $a from 10 to 200
var_dump($a);
// f(&$a);  // PHP5 32/62, Fatal error: Call-time pass-by-reference has been removed
         // HHVM accepts the & as being redundant
         // The php.net on-line help states: "As of PHP 5.3.0, you will get a warning
         // saying that "call-time pass-by-reference" is deprecated when you use & in
         // foo(&$a);. And as of PHP 5.4.0, call-time pass-by-reference was removed,
         // so using it will raise a fatal error."
var_dump($a);

f();     // So just what is f's $p aliased to? Presumably, nothing; $p is simply
         // undefined on entry to f. Then when assigned 200, $p becomes a local
         // variable that dies when it goes out of scope when the function terminates.
//*/

///*
// passing by reference with a default argument value

function g(&$p = "red")
{
   echo '$p '.(isset($p) ? "is set\n" : "is not set\n");
   echo "g In:  \$p: $p\n";
   $p = 200;      // actual argument's value changed
   echo "g Out: \$p: $p\n";
}

// pass a variable by reference; f changes its value

g();           // like the f() call above

$a = 10;
var_dump($a);
g($a);
var_dump($a);
//*/

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

///*
// create and use byRefs without involving functions

$a = 10;
$b = &$a;       // make $b an alias to $a
//var_dump(&$a);    // Call-time pass-byRefs has been removed
                // & really isn't an operator
//gettype(&$a); // Call-time pass-byRefs has been removed
var_dump($a);
var_dump($b);
echo "\n";

++$a;           // increment $a/$b to 11
var_dump($a);
var_dump($b);
echo "\n";

$b = -12;       // sets $a/$b to -12
var_dump($a);
var_dump($b);
echo "\n";

$a = "abc";     // sets $a/$b to "abc"
var_dump($a);
var_dump($b);
echo "\n";

$c = -100;
$b = &$c;       // make $b an alias to $c (instead of $a)
var_dump($a);
var_dump($b);
var_dump($c);
echo "\n";

$b = 1234;      // sets $b and $c to 1234
var_dump($a);
var_dump($b);
var_dump($c);
echo "\n";

unset($c);      // removes $c as an alias to $b
var_dump($b);
var_dump($c);
echo "\n";
//*/
///*
// create and use byRefs via the new operator

class C
{
    public $m;
}

unset($a);
var_dump($a);
$a = new C;     // make $a an alias to the allocated object
//$a = &new C;  // use of & here is deprecated
var_dump($a);
$a->m = "abc";
var_dump($a);
echo "\n";

$b = &$a;       // make $b an alias to $a
var_dump($a);
var_dump($b);
echo "\n";

$b->m = "xyz";  // change m in $a/$b
var_dump($a);
var_dump($b);
echo "\n";

unset($a);      // removes $a as an alias
var_dump($a);
var_dump($b);
//*/

///*
// pass byRef

function f(&$p)
{
    echo '$p '.(isset($p) ? "is set\n" : "is not set\n");
    echo "f In:  \$p: $p\n";
    $p = 200;       // actual argument's value changed
    echo "f Out: \$p: $p\n\n";
}

$a = 10;
var_dump($a);
f(&$a);   // change $a from 10 to 200
var_dump($a);
//*/

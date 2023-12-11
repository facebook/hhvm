<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

// a simple example of passing by value and by inout

function f($p1, inout $p2)
:mixed{
    echo "f In:  \$p1: $p1, \$p2: $p2\n";
    $p1 = 100;      // only the local copy is changed
    $p2 = 200;      // actual argument's value changed
    echo "f Out: \$p1: $p1, \$p2: $p2\n";
}

function g(inout $p1)
:mixed{
    $p1 = HH\is_any_array($p1) ? 'Array' : $p1;
    $p1__str = (string)($p1);
    echo "g In:  \$p1: $p1__str.\n";
    $p1 = 200;      // actual argument's value changed
    echo "g Out: \$p1: $p1.\n";
}
<<__EntryPoint>> function main(): void {
error_reporting(-1);

///*
$a1 = 10;
$a2 = 20;
var_dump($a1);
var_dump($a2);
f($a1, inout $a2);        // variable $a2 is passed by inout
var_dump($a1);
var_dump($a2);
$twenty = 20;
$a2 = vec[10,$twenty,30];
var_dump($a2);
f($a1, inout $twenty);
var_dump($a1);
var_dump($a2);










//*/

// passing by inout explored further

///*
$a2 = 0;
// g(TRUE);             // PHP5 32/64, Error: Only variables can be passed by reference
                           // HHVM,       Error: Cannot pass parameter 1 by reference
   $a2 = TRUE;
   g(inout $a2);              // OK; passing a modifiable lvalue by inout
   var_dump($a2);

// following tests have different values for $a2, and give results like the case above

   $a2 = -123;
   g(inout $a2);
var_dump($a2);
   $a2 = 1.23e3;
   g(inout $a2);
var_dump($a2);
   $a2 = NULL;
   g(inout $a2);
var_dump($a2);
   $a2 = "abc";
   g(inout $a2);
var_dump($a2);
   $a2 = vec[1,2,3];
   g(inout $a2);
var_dump($a2);
//*/

///*
// based on finding from above, further testing
$z = 10;
// ($z = 5) = 20;      // All 3, Error: unexpected '='
// ($z = 5)++;       // All 3, Error: unexpected '++'
// ($z += 5) = 20;  // All 3, Error: unexpected '='
// ($z += 5)++;     // All 3, Error: unexpected '++'
//*/

///*
$z = 10;
g(inout $z);              // create a reference to a modifiable lvalue
var_dump($z);
//*/
}

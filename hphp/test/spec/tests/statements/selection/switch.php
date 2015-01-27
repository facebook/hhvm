<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

$v = 10;

/*
// unlike C/C++, can't omit all labels and braces

switch ($v)
    echo "Here I am.\n";

switch ($v):
    echo "Here I am.\n";
endswitch;

switch ($v)
default: echo $v;
*/

///*
// Can I have an empty body?

switch ($v)     // yes
{
}

switch ($v):    // yes
endswitch;
//*/

///*
// presumably, body can be one or more empty statements

switch ($v)
{
    ;       // 1 is okay, but no more!
    //;
    //;
}
//*/

/*
switch ($v):    // No, can't have {} with alternate form
{
}
endswitch;
*/

///*
switch ($v)
{
default: echo "default case: \$v is $v\n";
}

switch ($v)
{
default:
    echo "default case: \$v is $v\n";
    break;      // break ends "group" of default statements
case 20:
    echo "case 20\n";
    break;      // break ends "group" of case 20 statements
case 10:
    echo "case 10\n";   // no break, so control drops into next label's "group"
case 30:
    echo "case 30\n";   // no break, but then none is really needed either
}

switch ($v):

default:
    echo "default case\n";
    break;
case 20:
    echo "case 20\n";
    break;
case 10:
    echo "case 10\n";
case 30:
    echo "case 30\n";

endswitch;
//*/

///*
// Check duplicate case values: allowed; choses lexically first one

$v = 30;
switch ($v)
{
case 30:
    echo "case 30-2\n";
    break;
default:
    echo "default case: \$v is $v\n";
    break;
case 30:
    echo "case 30-1\n";
    break;
}

// chooses first match with equal value, 30 matches 30.0 before 30

$v = 30;
switch ($v)
{
case 30.0:  // <===== this case matches with 30
    echo "case 30.0\n";
    break;
default:
    echo "default case: \$v is $v\n";
    break;
case 30:        // <===== rather than this case matching with 30
    echo "case 30\n";
    break;
}
//*/

///*
// ; is allowed in place of : at end of case/default label; can mix-n-match

$v = 10;
switch ($v)
{
case 10;        // <================ ;
    echo "case 10\n";
    break;
case 20:        // <================ :
    echo "case 20\n";
    break;
default;        // <================ ;
    echo "default case: \$v is $v\n";
    break;
}

$v = 10;
switch ($v):

case 10;        // <================ ;
    echo "case 10\n";
    break;
case 20:        // <================ :
    echo "case 20\n";
    break;
default;        // <================ ;
    echo "default case: \$v is $v\n";
    break;

endswitch;
//*/

///*
// use  strings for label values

$v = "white";
switch ($v)
{
case "White":
    echo "case White\n";
    break;
case "Red":
    echo "case Red\n";
    break;
default:
    echo "default case: \$v is $v\n";
    break;
}

// use Booleans for label values

$v = TRUE;
switch ($v)
{
case FALSE:
    echo "case FALSE\n";
    break;
case TRUE:
    echo "case TRUE\n";
    break;
default:
    echo "default case: \$v is $v\n";
    break;
}
//*/

// use Booleans for label values

///*
$v = 22;
$a = 1;
$b = 12;
switch ($v)
{
case 10 + $b:
    echo "case 10 + $b\n";
    break;
case $v < $a:
    echo "case $v < $a\n";
    break;
default:
    echo "default case: \$v is $v\n";
    break;
}
//*/

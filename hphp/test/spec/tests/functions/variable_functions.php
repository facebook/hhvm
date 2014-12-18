<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

include_once 'TestInc.inc'; // get access to \NS1\f2()

function f1()
{
    echo "Inside function " . __FUNCTION__ . "\n";
}

f1();
\f1();
namespace\f1();

$v = 'f1';
$v();
$v = '\\f1';
$v();
//'f1'();           // can't be a literal

$v = '\\NS1\\f2';
$v();
//'\\NS1\\f2'();    // can't be a literal

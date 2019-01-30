<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

function cleanup1()
{
    echo "Inside " . __METHOD__ . "\n";
}

function cleanup2()
{
    echo "Inside " . __METHOD__ . "\n";
}

register_shutdown_function('cleanup2');
register_shutdown_function('cleanup1');

echo "--------- test with/without string -------------\n";

include_once 'Point2.inc';

$p1 = new Point2(5, 3);
$p2 = new Point2;
$p3 = new Point2;

exit("goodbye\n");  // writes "goodbye", then destructors are called.
//exit(99);         // writes nothing
//exit();           // writes nothing
//exit;             // writes nothing

echo "end of script\n";

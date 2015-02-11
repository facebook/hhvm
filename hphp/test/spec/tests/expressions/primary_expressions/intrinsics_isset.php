<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

echo "--------- TRUE -------------\n";

$v = TRUE;
var_dump(isset($v));

echo "--------- NULL -------------\n";

$v = NULL;
var_dump(isset($v));

echo "--------- TRUE, 12.3, NULL -------------\n";

$v1 = TRUE; $v2 = 12.3; $v3 = NULL;
var_dump(isset($v1, $v2, $v3));

echo "--------- undefined parameter -------------\n";

function f($p)
{
    var_dump($p);
    var_dump(isset($p));
}

f();
f(NULL);
f(10);

echo "---------- dynamic property ------------\n";

class X1
{
}

class X2
{
    public function __isset($name)
    {
        echo "Inside " . __METHOD__ . " with \$name $name\n";
        return FALSE;
//      return TRUE;
    }
}

$x1 = new X1;
var_dump(isset($x1->m));
$x1->m = 123;
var_dump(isset($x1->m));

$x2 = new X2;
var_dump(isset($x2->m));

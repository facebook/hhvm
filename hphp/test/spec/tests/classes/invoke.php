<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class C
{
///*
    public function __invoke($p)
    {
        echo "Inside " . __METHOD__ . " with arg $p\n";

        return "xxx";
    }
//*/
}

$c = new C;
var_dump(is_callable($c)); // returns TRUE is __invoke exists; otherwise, FALSE
$r = $c(123);
var_dump($r);
$r = $c("Hello");
var_dump($r);

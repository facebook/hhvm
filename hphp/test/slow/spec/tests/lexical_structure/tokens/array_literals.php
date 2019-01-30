<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class X
{
    private $prop10 = array();
    private $prop11 = array(10, "a" => "red", TRUE);
    private $prop12 = array(10, "red", "xx" => array(2.3, NULL, "zz" => array(12, FALSE, "zzz")));

    private $prop20 = [];
    private $prop21 = [10, "a" => "red", TRUE];
    private $prop22 = [10, "red", "xx" => [2.3, NULL, "zz" => [12, FALSE, "zzz"]]];
}

$x = new X;
var_dump($x);

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

include_once 'MyRangeException.inc';

$re = new MyRangeException("xxx", 5, 20, 30);
var_dump($re);

echo "=======\n";

echo "\$re = >$re<\n";

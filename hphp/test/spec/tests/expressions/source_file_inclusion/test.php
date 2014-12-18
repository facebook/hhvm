<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

function test()
{
    echo "Inside test() in " . __FILE__ . "\n";
    echo "\$v1: $v1, \$v2: $v2\n";
}

$local1 = 100;
var_dump($local1);

echo "====\n";
print_r(get_included_files());
echo "====\n";

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

// use recursion to implement a factorial function
// Note: can call a function prior to its definition

for ($i = 0; $i <= 10; ++$i)
{
    printf("%2d! = %d\n", $i, factorial($i));
}

function factorial($int)
{
    return ($int > 1) ? $int * factorial($int - 1) : $int;
}

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

$i = 1;
do
{
    echo "$i\t".($i * $i)."\n"; // output a table of squares
    ++$i;
}
while ($i <= 10);

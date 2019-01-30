<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

$i = 1;
while ($i <= 10)
{
    echo "$i\t".($i * $i)."\n";
    ++$i;
}

$i = 1;
while ($i <= 10):
    echo "$i\t".($i * $i)."\n"; // output a table of squares
    ++$i;
endwhile;

$count = 0;
while (TRUE)
{
    if (++$count == 5)
        $done = TRUE;
    echo $count."\n";
    // ...
    if ($done)
        break;  // break out of the while loop
    // ...
}

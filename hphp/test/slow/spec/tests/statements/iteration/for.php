<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

for ($i = 1; $i <= 10; ++$i)
{
    echo "$i\t".($i * $i)."\n"; // output a table of squares
}

// omit 1st and 3rd expressions

$i = 1;
for (; $i <= 10;):
    echo "$i\t".($i * $i)."\n"; // output a table of squares
    ++$i;
endfor;

// omit all 3 expressions

$i = 1;
for (;;)
{
    if ($i > 10)
        break;
    echo "$i\t".($i * $i)."\n"; // output a table of squares
    ++$i;
}

//  use groups of expressions

for ($a = 100, $i = 1; ++$i, $i <= 10; ++$i, $a -= 10)
{
    echo "$i\t$a\n";
}

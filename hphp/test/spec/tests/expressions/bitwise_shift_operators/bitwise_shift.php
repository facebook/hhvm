<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

$i32 = 1 << 31; // if this is negative, we have a 32-bit int
$NumBitsPerInt = ($i32 < 0) ? 32 : 64;

// Shift a positive value right and left using both in- and out-of-range counts

$v = 1000;
for ($i = -$NumBitsPerInt - 1; $i <= $NumBitsPerInt + 1; ++$i)
{
    printf("%d(%08X): >> %2d = %08X\t<< %2d = %08X\n", $v, $v, $i, $v >> $i, $i, $v << $i);
}

// Shift a negative value right and left using both in- and out-of-range counts

$v = -1000;
for ($i = -$NumBitsPerInt - 1; $i <= $NumBitsPerInt + 1; ++$i)
{
    printf("%d(%08X): >> %2d = %08X\t<< %2d = %08X\n", $v, $v, $i, $v >> $i, $i, $v << $i);
}

// Shift all kinds of scalar values to see which are ints or can be implicirly converted

$scalarValueList = array(10, -100, 0, 1.234, 0.0, TRUE, FALSE, NULL, "123", 'xx', "");
foreach ($scalarValueList as $v)
{
    printf("%d(%08X): >> %2d = %08X\t<< %2d = %08X\n", $v, $v, 3, $v >> 3, 5, $v << 5);
}

// Figure out the algorithm the implementations use for negative and too-large shift counts

for ($i = -129; $i <= 129; ++$i)
{
    $rem = $i % $NumBitsPerInt;
    if ($rem == 0 || $i > 0)
    {
        echo "$i, ".$rem."\n";
    }
    else    // have a negative shift
    {
        $r = $NumBitsPerInt - (-$i % $NumBitsPerInt);
        echo "$i, ".$r."\n";
    }
}

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

require 'bitwise_shift_helper.php';

error_reporting(-1);

$i32 = 1 << 31; // if this is negative, we have a 32-bit int
$NumBitsPerInt = ($i32 < 0) ? 32 : 64;

// Shift a positive value right and left using both in- and out-of-range counts

$v = 1000;
printShiftRange($v, -$NumBitsPerInt - 1, -1);
printShiftRange($v, $NumBitsPerInt, $NumBitsPerInt + 1);

// Shift a negative value right and left using both in- and out-of-range counts

$v = -1000;
printShiftRange($v, -$NumBitsPerInt - 1, -1);
printShiftRange($v, $NumBitsPerInt, $NumBitsPerInt + 1);

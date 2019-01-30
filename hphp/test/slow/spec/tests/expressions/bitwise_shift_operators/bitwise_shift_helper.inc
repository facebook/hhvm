<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

function printShiftRange($v, $start, $end)
{
    for ($i = $start; $i <= $end; ++$i)
    {
        printf("%d(%08X): >> %2d = %08X\t<< %2d = %08X\n",
               $v, $v, $i, $v >> $i, $i, $v << $i);
    }
}

?>

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

function DoIt() { return 20; }

$i = 10;// $i is assigned the value 10; result (10) is discarded
++$i;   // $i is incremented; result (11) is discarded
$i++;   // $i is incremented; result (11) is discarded
DoIt(); // function DoIt is called; result (return value) is discarded

$i;     // no side effects; result is discarded, so entirely vacuous
123;    // likewise ...
34.5 * 12.6 + 11.987;
TRUE;

while ($i-- > 0)
{
    ;       // null statement
}

$i = 10;
while ($i-- > 0)
{
    continue;   // in this context, equivalent to using a null statement
}

$table = array();
$table[0][0] = 34;
$table[0][1] = -3;
$table[0][2] = 345;
$table[1][0] = 123;
$table[1][1] = 9854;
$table[1][2] = -765;

function findValue($table, $v)  // where $table is 2x3 array
{
    for ($row = 0; $row <= 1; ++$row)
    {
        for ($colm = 0; $colm <= 2; ++$colm)
        {
            if ($table[$row][$colm] == $v)
            {
                echo "$v was found at row $row, column $colm\n";
                goto done; // not quite the same as break 2!
            }
        }
    }

    echo "$v was not found\n";
done:
    ;       // note that a label must always precede a statement
}

findValue($table, 123);
findValue($table, -23);

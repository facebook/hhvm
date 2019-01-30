<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

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

/*
// Try to jump into a while loop (which is inside a for loop); disallowed

goto label1;
for($i = 1; $j = 20, $i <= 10; $i++)
{
        while($j--)
        {
label1:
                ;
        }
}
*/

<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
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
                                break 2; // yes, I know it goes to the wrong place
                        }
                }
        }

        echo "$v was not found\n";
done:
        ;
}

findValue($table, 123);
findValue($table, -23);

// break;  // can't break from the outer-most level

function f($i)
{
        echo "$i\n";
        break;  // break is not rejected here until runtime
}

//f(12);        // fails

for ($i = 1; $i <= 3; ++$i)
{
//        f($i);        // fails
}

for ($i = 10; $i <= 40; $i +=10)
{
        echo "\n\$i = $i: ";
        switch($i)
        {
        case 10: echo "ten"; break;
        case 20: echo "twenty"; break 2;
        case 30: echo "thirty"; break;
        }
        echo "\nJust beyond the switch";
}
echo "\n----------\n";

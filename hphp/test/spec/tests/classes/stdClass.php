<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(E_ALL^ E_WARNING);

function displayClass($o)
{
//  var_dump($o);
    echo "\nType is >" . get_class($o) . "<\n";
    $a = (array)$o;
//  var_dump($a);
    if (count($a) == 0)
    {
        echo "    Instance has no properties\n";
    }
    else
    {
        foreach ($a as $name => $value)
        {
            echo "    Property >$name< with value $value\n";
        }
    }
}

echo "====== explicit conversions of certain values to type object ======\n";

displayClass((object)NULL);
displayClass((object)10);
displayClass((object)array(11, 21, 31));

echo "\n====== Use of certain values as the left-hand operand of -> ======\n";

$p = FALSE;
$p->x = 33;
displayClass($p);

$p = NULL;
$p->x = 44;
displayClass($p);

$p = "";
$p->x = 55;
displayClass($p);

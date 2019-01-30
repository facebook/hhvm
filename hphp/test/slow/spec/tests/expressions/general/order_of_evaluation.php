<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

///*
function f($a, $b)
{
    echo "Inside f: \$a = $a, \$b = $b\n";
    return 22;
}

function g($a)
{
    echo "Inside g: \$a = $a\n";
    return 10;
}
//*/

///*
$i = 10;
echo "\$i = $i: "; echo '$i++ + $i = '.($i++ + $i)."\t\t"; echo "\$i = $i\n";
$i = 10;
echo "\$i = $i: "; echo '$i - $i-- = '.($i - $i--)."\t\t"; echo "\$i = $i\n";
$i = 10;
echo "\$i = $i: "; echo '$++i * $i = '.(++$i * $i)."\t"; echo "\$i = $i\n";
$i = 10;
echo "\$i = $i: "; echo '$i / --$i = '.($i / --$i)."\t\t"; echo "\$i = $i\n";
var_dump(10/9);
//*/

///*
$values = array(0, 1, 2, 3, 4, 5, 6);
$i = 3;
echo "\n".'initially, $i = '.$i."\n";
$values[$i] = $values[++$i];        // prefix ++
echo '  finally, $i = '.$i."\n";
foreach ($values as $a)
{
    echo "$a ";
}
echo "\n";

$values = array(0, 1, 2, 3, 4, 5, 6);
$i = 3;
echo "\n".'initially, $i = '.$i."\n";
$values[$i] = $values[$i++];        // postfix ++
echo '  finally, $i = '.$i."\n";
foreach ($values as $a)
{
    echo "$a ";
}
echo "\n";
//*/

///*
$values = array(0, 1, 2, 3, 4, 5, 6);
$i = 3;
echo "\n".'initially, $i = '.$i."\n";
$values[$i] = f($i, ++$i);
echo '  finally, $i = '.$i."\n";
foreach ($values as $a)
{
    echo "$a ";
}
echo "\n";

$values = array(0, 1, 2, 3, 4, 5, 6);
$i = 3;
echo "\n".'initially, $i = '.$i."\n";
$values[$i] *= f($i, ++$i);
echo '  finally, $i = '.$i."\n";
foreach ($values as $a)
{
    echo "$a ";
}
echo "\n";

$values = array(0, 1, 2, 3, 4, 5, 6);
$i = 3;
echo "\n".'initially, $i = '.$i."\n";
$values[$i] = f($i, ++$i) + g(++$i);
echo '  finally, $i = '.$i."\n";
foreach ($values as $a)
{
    echo "$a ";
}
echo "\n";

$values = array(0, 1, 2, 3, 4, 5, 6);
$i = 3;
echo "\n".'initially, $i = '.$i."\n";
$values[$i] = f($i, ++$i) - g(++$i);
echo '  finally, $i = '.$i."\n";
foreach ($values as $a)
{
    echo "$a ";
}
echo "\n";

$values = array(0, 1, 2, 3, 4, 5, 6);
$i = 3;
echo "\n".'initially, $i = '.$i."\n";
$values[$i] = f($i, ++$i) / g(++$i) - f(--$i, $i);
echo '  finally, $i = '.$i."\n";
foreach ($values as $a)
{
    echo "$a ";
}
echo "\n";

$values = array(0, 1, 2, 3, 4, 5, 6);
$i = 3;
echo "\n".'initially, $i = '.$i."\n";
$values[$i] = f($i, ++$i) + g(++$i) + f(--$i, $i) + ($i = 5);
echo '  finally, $i = '.$i."\n";
foreach ($values as $a)
{
    echo "$a ";
}
echo "\n";
//*/

echo 'f(10, 12) + g(15) = '.(f(10, 12) + g(15))."\n";
echo 'f(10, 12) - g(15) = '.(f(10, 12) - g(15))."\n";
echo 'f(10, 12) * g(15) = '.(f(10, 12) * g(15))."\n";
echo 'f(10, 12) / g(15) = '.(f(10, 12) / g(15))."\n";

function f1($a)
{
    echo "Inside f1\n";
    return $a;
}

function f2($a)
{
    echo "Inside f2\n";
    return $a;
}

function f3($a)
{
    echo "Inside f3\n";
    return $a;
}

function f4($a)
{
    echo "Inside f4\n";
    return $a;
}

$values = array(0, 1, 2, 3, 4, 5, 6);
var_dump($values);
$values[f1(4) - f2(2)] = $values[f3(3) * f4(2)];
var_dump($values);
$values = array(0, 1, 2, 3, 4, 5, 6);
$values[f1(1) + f2(2)] = $values[f3(6) / f4(3)];
var_dump($values);

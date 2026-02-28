<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

$array = vec[];
foreach ($array as $a)
{
    echo $a."\n";
}

$colors = vec["red", "white", "blue"];

// access each element's value

foreach ($colors as $color)
{
    echo $color."\n";
}
echo $color."\n";

// access each element's value anf its element number


foreach ($colors as $index => $color)
{
    echo "Index: $index; Color: $color\n";
    var_dump($index);
}
echo "Index: $index; Color: $color\n";

// Modify the local copy of an element's value

foreach ($colors as $color)
{
    echo $color."\n";
    $color = "black";
    echo $color."\n";
}
var_dump($colors);

$ary = dict[];
$ary[0] = vec["abc"];
$ary[0][] = "ij";
$ary[1] = vec["mnop"];
$ary[1][] = "xyz";

foreach ($ary as $e1)
{
    foreach ($e1 as $e2)
    {
        echo "  $e2";
    }
    echo "\n";
}

// test use of list

$a = vec[vec[10,20], vec[1.2, 4.5], vec[TRUE, "abc"]];
foreach ($a as $key => $value)
{
    echo "------\n";
    var_dump($key);
    var_dump($value);
}

foreach ($a as $key => list($v1, $v2))
{
    echo "------\n";
    var_dump($key);
    var_dump($v1);
    var_dump($v2);
}
}

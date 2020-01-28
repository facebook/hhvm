<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

echo "================= array of zero elements is possible =================\n";

$v = darray[];
var_dump($v);
$v = darray[];
var_dump($v);

echo "================= array of 1 element is possible =================\n";

$v = varray[TRUE];
var_dump($v);
$v = varray[TRUE];
var_dump($v);
$v = darray[0 => TRUE];  // specify explicit key
var_dump($v);
$v = darray[0 => TRUE];
var_dump($v);

echo "================= array of 2 elements each having the same type =================\n";

$v = varray[123, -56];
var_dump($v);
$v = varray[123, -56];
var_dump($v);
$v = darray[0 => 123, 1 => -56]; // specify explicit keys
var_dump($v);
$v = darray[0 => 123, 1 => -56];
var_dump($v);

$pos = 1;
$v = darray[0 => 123, $pos => -56];  // specify explicit keys
var_dump($v);
$v = darray[0 => 123, $pos => -56];       // key can be a variable
var_dump($v);

$i = 10;
$v = darray[0 => 123, $pos => -56];  // specify explicit keys
var_dump($v);
$v = darray[$i - 10 => 123, $i - 9 => -56];   // key can be a runtime expression
var_dump($v);

echo "================= array of 5 elements each having different type =================\n";

$v = varray[NULL, FALSE, 123, 34e12, "Hello"];
var_dump($v);
$v = varray[NULL, FALSE, 123, 34e12, "Hello"];
var_dump($v);
$v = darray[0 => NULL, 1 => FALSE, 2 => 123, 3 => 34e12, 4 => "Hello"];
var_dump($v);
$v = darray[0 => NULL, 1 => FALSE, 2 => 123, 3 => 34e12, 4 => "Hello"];
var_dump($v);
$v = array(NULL, 1 => FALSE, 123, 3 => 34e12, "Hello"); // some keys default, others not
var_dump($v);
$v = [NULL, 1 => FALSE, 123, 3 => 34e12, "Hello"];
var_dump($v);

echo "================= trailing comma permitted if list has at least one entry =================\n";

// $v = array(,);   // error
// $v = [,];        // error

$v = varray[TRUE,];
var_dump($v);
$v = varray[TRUE,];
var_dump($v);
$v = darray[0 => TRUE,];
var_dump($v);
$v = darray[0 => TRUE,];
var_dump($v);

$v = varray[123, -56,];
var_dump($v);
$v = varray[123, -56,];
var_dump($v);
$v = darray[0 => 123, 1 => -56,];
var_dump($v);
$v = darray[0 => 123, 1 => -56,];
var_dump($v);

echo "================= specify keys in arbitrary order, initial values of runtime expressions, leave gaps =================\n";

$i = 6;
$j = 12;
$v = darray[7 => 123, 3 => $i, 6 => ++$j];
var_dump($v);

$i = 6;
$j = 12;
$v = darray[7 => 123, 3 => $i, 6 => ++$j];
var_dump($v);

foreach($v as $e)   // only has 3 elements ([3], [6], and [7]), not 8 ([0]-[7])
{
    echo $e.' ';
}
echo "\n";

// access non-existant element
try { echo "\$v[1] is >".$v[1]."<\n"; } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump($v1[1]); } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { echo "\$v[4] is >".$v[4]."<\n"; } catch (Exception $e) { echo $e->getMessage()."\n"; }
try { var_dump($v1[4]); } catch (Exception $e) { echo $e->getMessage()."\n"; }

$v[1] = TRUE;       // increases array to 4 elements
$v[4] = 99;         // increases array to 5 elements
var_dump($v);
foreach($v as $e)       // now has 5 elements
{
    echo $e.' ';
}
echo "\n";

echo "================= duplicate keys allowed, but lexically final one used =================\n";

$v = darray[2 => 23, 1 => 10, 2 => 46, 1.9 => 6];    // key 1.9 is truncated to key 1
var_dump($v);

echo "================= string keys can be expressions too =================\n";

$s1 = "color";
$s2 = "shape";
$v = darray[$s1 => "red", $s2 => "square"];
var_dump($v);

echo "================= can mix int and string keys =================\n";

// "4" as key is taken as key 4
// 9.2 as key is truncated to key 9
// "12.8" as key is treated as key with that string, NOT truncated and made int 12
// NULL as key becomes key ""

$v = darray["red" => 10, "4" => 3, 9.2 => 5, "12.8" => 111, NULL => 1];
var_dump($v);

$v = darray[FALSE => -4];    // FALSE as key becomes key 0
var_dump($v);
$v = darray["" => -3];
var_dump($v);
$v = darray[INF => 21];  // INF as key becomes key 0/IntMin/0 (imp-def?)
var_dump($v);
$v = darray[-INF => -1]; // -INF as key becomes key 0/IntMin/IntMin (imp-def?)
var_dump($v);
$v = darray[NAN => 123]; // NAN as key becomes key of IntMin/IntMin/IntMin (imp-def?)
var_dump($v);

echo "================= arrays some of whose elements are arrays, and so on =================\n";

$c = varray["red", "white", "blue"];
$v = varray[10, $c, NULL, varray[FALSE, NULL, $c]];
var_dump($v);

$v = varray[varray[2,4,6,8], varray[5,10], varray[100,200,300]];
var_dump($v);

echo "================= see if int keys can be specified in any base. =================\n";

$v = darray[12 => 10, 0x10 => 16, 010 => 8, 0b11 => 2];
var_dump($v);

echo "================= what about int-looking strings? It appears not. =================\n";

$v = darray["12" => 10, "0x10" => 16, "010" => 8, "0b11" => 2];
var_dump($v);

echo "================= iterate using foreach and compare with for loop =================\n";

$v = darray[2 => TRUE, 0 => 123, 1 => 34.5, -1 => "red"];
var_dump($v);
foreach($v as $e)
{
    echo $e.' ';
}
echo "\n";
for ($i = -1; $i <= 2; ++$i)
{
    echo $v[$i].' ';
}
echo "\n";

echo "================= remove some elements from an array =================\n";

$v = array("red" => TRUE, 123, 9 => 34e12, "Hello");
var_dump($v);
unset($v[0], $v["red"]);
var_dump($v);
}

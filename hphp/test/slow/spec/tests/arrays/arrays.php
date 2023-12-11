<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

echo "================= array of zero elements is possible =================\n";

$v = dict[];
var_dump($v);
$v = dict[];
var_dump($v);

echo "================= array of 1 element is possible =================\n";

$v = vec[TRUE];
var_dump($v);
$v = vec[TRUE];
var_dump($v);
$v = dict[0 => TRUE];  // specify explicit key
var_dump($v);
$v = dict[0 => TRUE];
var_dump($v);

echo "================= array of 2 elements each having the same type =================\n";

$v = vec[123, -56];
var_dump($v);
$v = vec[123, -56];
var_dump($v);
$v = dict[0 => 123, 1 => -56]; // specify explicit keys
var_dump($v);
$v = dict[0 => 123, 1 => -56];
var_dump($v);

$pos = 1;
$v = dict[0 => 123, $pos => -56];  // specify explicit keys
var_dump($v);
$v = dict[0 => 123, $pos => -56];       // key can be a variable
var_dump($v);

$i = 10;
$v = dict[0 => 123, $pos => -56];  // specify explicit keys
var_dump($v);
$v = dict[$i - 10 => 123, $i - 9 => -56];   // key can be a runtime expression
var_dump($v);

echo "================= array of 5 elements each having different type =================\n";

$v = vec[NULL, FALSE, 123, 34e12, "Hello"];
var_dump($v);
$v = vec[NULL, FALSE, 123, 34e12, "Hello"];
var_dump($v);
$v = dict[0 => NULL, 1 => FALSE, 2 => 123, 3 => 34e12, 4 => "Hello"];
var_dump($v);
$v = dict[0 => NULL, 1 => FALSE, 2 => 123, 3 => 34e12, 4 => "Hello"];
var_dump($v);
$v = dict[0 => NULL, 1 => FALSE, 2 => 123, 3 => 34e12, 4 => "Hello"]; // some keys default, others not
var_dump($v);
$v = dict[0 => NULL, 1 => FALSE, 2 => 123, 3 => 34e12, 4 => "Hello"];
var_dump($v);

echo "================= trailing comma permitted if list has at least one entry =================\n";

// $v = array(,);   // error
// $v = [,];        // error

$v = vec[TRUE,];
var_dump($v);
$v = vec[TRUE,];
var_dump($v);
$v = dict[0 => TRUE,];
var_dump($v);
$v = dict[0 => TRUE,];
var_dump($v);

$v = vec[123, -56,];
var_dump($v);
$v = vec[123, -56,];
var_dump($v);
$v = dict[0 => 123, 1 => -56,];
var_dump($v);
$v = dict[0 => 123, 1 => -56,];
var_dump($v);

echo "================= specify keys in arbitrary order, initial values of runtime expressions, leave gaps =================\n";

$i = 6;
$j = 12;
$v = dict[7 => 123, 3 => $i, 6 => ++$j];
var_dump($v);

$i = 6;
$j = 12;
$v = dict[7 => 123, 3 => $i, 6 => ++$j];
var_dump($v);

foreach($v as $e)   // only has 3 elements ([3], [6], and [7]), not 8 ([0]-[7])
{
    echo $e.',';
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
    echo (string)($e).',';
}
echo "\n";

echo "================= duplicate keys allowed, but lexically final one used =================\n";

$v = dict[2 => 23, 2 => 46, 2 => 6];
var_dump($v);

echo "================= string keys can be expressions too =================\n";

$s1 = "color";
$s2 = "shape";
$v = dict[$s1 => "red", $s2 => "square"];
var_dump($v);

echo "================= arrays some of whose elements are arrays, and so on =================\n";

$c = vec["red", "white", "blue"];
$v = vec[10, $c, NULL, vec[FALSE, NULL, $c]];
var_dump($v);

$v = vec[vec[2,4,6,8], vec[5,10], vec[100,200,300]];
var_dump($v);

echo "================= see if int keys can be specified in any base. =================\n";

$v = dict[12 => 10, 0x10 => 16, 010 => 8, 0b11 => 2];
var_dump($v);

echo "================= what about int-looking strings? It appears not. =================\n";

$v = dict["12" => 10, "0x10" => 16, "010" => 8, "0b11" => 2];
var_dump($v);

echo "================= iterate using foreach and compare with for loop =================\n";

$v = dict[2 => TRUE, 0 => 123, 1 => 34.5, -1 => "red"];
var_dump($v);
foreach($v as $e)
{
    echo (string)($e).',';
}
echo "\n";
for ($i = -1; $i <= 2; ++$i)
{
    echo (string)($v[$i]).',';
}
echo "\n";

echo "================= remove some elements from an array =================\n";

$v = dict["red" => TRUE, 0 => 123, 9 => 34e12, 10 => "Hello"];
var_dump($v);
unset($v[0], $v["red"]);
var_dump($v);
}

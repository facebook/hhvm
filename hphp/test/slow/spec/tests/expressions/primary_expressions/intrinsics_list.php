<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

echo "--------- test with full and omitted LHS vars -------------\n";
$v = vec[0, 100, 67];
list($min, $max, $avg) = $v;
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);
$v = dict[2 => 67, 1 => 100, 0 => 0];
list($min, $max, $avg) = $v;
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

list($min, , $avg) = vec[0, 100, 67];
echo "\$min: $min, , \$avg: $avg\n";

list(, $max, $avg) = vec[0, 100, 67];
echo ", \$max: $max, \$avg: $avg\n";

list(, , $avg) = vec[0, 100, 67];
echo ", , \$avg: $avg\n";

list($min, $max, ) = vec[0, 100, 67];
echo "\$min: $min, \$max: $max,\n";

list($min, $max) = vec[0, 100, 67];
echo "\$min: $min, \$max: $max\n";

list($min, , ) = vec[0, 100, 67];
echo "\$min: $min, ,\n";

list($min) = vec[0, 100, 67];
echo "\$min: $min\n";

echo "--------- test with more array elements than variables -------------\n";
$v = vec[0, 100, 67, 22, 33];
list($min, $max, $avg) = $v;
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

echo "--------- test with fewer array elements than variables -------------\n";

var_dump(isset($min));
var_dump($min);
var_dump(isset($max));
var_dump($max);
var_dump(isset($avg));
var_dump($avg);

try {
    $v = vec[100, 500];
    list($min, $max, $avg) = $v; //Undefined offset: 2
} catch (Exception $e) { echo $e->getMessage()."\n"; }
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

var_dump(isset($min));  // TRUE
var_dump($min);
var_dump(isset($max));  // TRUE
var_dump($max);
var_dump(isset($avg));  // FALSE
var_dump($avg);

echo "--------- test with sufficient array elements but not consecutive keys -------------\n";

try {
     $v = dict[0 => 0, 2 => 100, 4 => 67];
     list($min, $max, $avg) = $v;
} catch (Exception $e) { echo $e->getMessage()."\n"; }
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

var_dump(isset($min));  // TRUE
var_dump($min);
var_dump(isset($max));  // FALSE
var_dump($max);
var_dump(isset($avg));  // TRUE
var_dump($avg);

echo "--------- test with NULL rather than array -------------\n";

//$v = list($min, $max, $avg);  // syntax error, unexpected ';', expecting '='

$v = NULL;
list($min, $max, $avg) = $v;
var_dump(isset($v));    // FALSE

echo "--------- test with mixed array -------------\n";
$v = dict[0 => 10, "a" => 20, 1 => 30, "b" => 40, 2 => 50];
list($min, $max, $avg) = $v;
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

echo "--------- test with non-numeric array -------------\n";

try {
    $v = dict["x" => 10, "a" => 20, "y" => 30];
    list($min, $max, $avg) = $v;
} catch (Exception $e) { echo $e->getMessage()."\n"; }
    // Undefined offset: 2, 1, 0
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

var_dump(isset($min));  // FALSE
var_dump(isset($max));  // FALSE
var_dump(isset($avg));  // FALSE

echo "--------- test with array element being an array -------------\n";

try {
    $v = vec[0, vec[100, 67]];
    list($min, $max, $avg) = $v; // Undefined offset: 2
} catch (Exception $e) { echo $e->getMessage()."\n"; }
print_r($v);

var_dump(isset($min));  // TRUE
var_dump($min);
var_dump(isset($max));  // TRUE
var_dump($max);         // array(100, 67)
var_dump(isset($avg));  // FALSE
var_dump($avg);

echo "--------- test with nested lists -------------\n";
$v = vec[0, dict[1 => 67, 2 => 99, 0 => 100], 33]; list($min, list($max, $avg)) = $v;
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

echo "--------- test with target vars being array elements -------------\n";
$a = dict[];
$v = vec[0, 100, 67]; list($a[0], $a[2], $a[4]) = $v;
print_r($a);
print_r($v);

echo "--------- test with no variables -------------\n";
$v =  vec[0, 100, 67];
list() = $v;
print_r($v);
$v = vec[0, 100, 67];
list(,) = $v;
print_r($v);
$v = vec[0, 100, 67];
list(,,) = $v;
print_r($v);
}

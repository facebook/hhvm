<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

echo "--------- test with full and omitted LHS vars -------------\n";

$v = list($min, $max, $avg) = varray[0, 100, 67];
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

$v = list($min, $max, $avg) = darray[2 => 67, 1 => 100, 0 => 0];
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

list($min, , $avg) = varray[0, 100, 67];
echo "\$min: $min, , \$avg: $avg\n";

list(, $max, $avg) = varray[0, 100, 67];
echo ", \$max: $max, \$avg: $avg\n";

list(, , $avg) = varray[0, 100, 67];
echo ", , \$avg: $avg\n";

list($min, $max, ) = varray[0, 100, 67];
echo "\$min: $min, \$max: $max,\n";

list($min, $max) = varray[0, 100, 67];
echo "\$min: $min, \$max: $max\n";

list($min, , ) = varray[0, 100, 67];
echo "\$min: $min, ,\n";

list($min) = varray[0, 100, 67];
echo "\$min: $min\n";

echo "--------- test with more array elements than variables -------------\n";

$v = list($min, $max, $avg) = varray[0, 100, 67, 22, 33];
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

echo "--------- test with fewer array elements than variables -------------\n";

var_dump(isset($min));
var_dump($min);
var_dump(isset($max));
var_dump($max);
var_dump(isset($avg));
var_dump($avg);

try { list($min, $max, $avg) = $v = varray[100, 500];  // Undefined offset: 2
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

try { list($min, $max, $avg) = $v = darray[0 => 0, 2 => 100, 4 => 67];
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

$v = list($min, $max, $avg) = NULL;
var_dump(isset($v));    // FALSE

echo "--------- test with mixed array -------------\n";

$v = list($min, $max, $avg) = darray[0 => 10, "a" => 20, 1 => 30, "b" => 40, 2 => 50];
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

echo "--------- test with non-numeric array -------------\n";

try {
list($min, $max, $avg) = $v = darray["x" => 10, "a" => 20, "y" => 30];
} catch (Exception $e) { echo $e->getMessage()."\n"; }
    // Undefined offset: 2, 1, 0
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

var_dump(isset($min));  // FALSE
var_dump(isset($max));  // FALSE
var_dump(isset($avg));  // FALSE

echo "--------- test with array element being an array -------------\n";

try { list($min, $max, $avg) = $v = varray[0, varray[100, 67]]; // Undefined offset: 2
} catch (Exception $e) { echo $e->getMessage()."\n"; }
print_r($v);

var_dump(isset($min));  // TRUE
var_dump($min);
var_dump(isset($max));  // TRUE
var_dump($max);         // array(100, 67)
var_dump(isset($avg));  // FALSE
var_dump($avg);

echo "--------- test with nested lists -------------\n";

$v = list($min, list($max, $avg)) = varray[0, darray[1 => 67, 2 => 99, 0 => 100], 33];
echo "\$min: $min, \$max: $max, \$avg: $avg\n";
print_r($v);

echo "--------- test with target vars being array elements -------------\n";
$a = darray[];
$v = list($a[0], $a[2], $a[4]) = varray[0, 100, 67];
print_r($a);
print_r($v);

echo "--------- test with no variables -------------\n";

$v = list() = varray[0, 100, 67];
print_r($v);

$v = list(,) = varray[0, 100, 67];
print_r($v);

$v = list(,,) = varray[0, 100, 67];
print_r($v);
}

<?hh
<<__EntryPoint>> function main(): void {
echo "\n*** Testing array_keys() on range of values ***\n";
$arr_range = dict[
  2147483647 => 1,
  2147483648 => 2,
  -2147483647 => 3, 
  -2147483648 => 4,
  -2147483649 => 5,
  -0 => 6,
  0 => 7
];
var_dump(array_keys($arr_range));

echo "\n*** Testing array_keys() on an array created on the fly ***\n";
var_dump(array_keys(dict["a" => 1, "b" => 2, "c" => 3]));
var_dump(array_keys(vec[]));  // null array

echo "Done\n";
}

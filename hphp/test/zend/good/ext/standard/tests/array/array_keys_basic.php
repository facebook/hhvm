<?hh
<<__EntryPoint>> function main(): void {
echo "*** Testing array_keys() on basic array operation ***\n";
$basic_arr = array("a" => 1, "b" => 2, 2.0 => 2.0, -23.45 => "asdasd",
                   varray[1,2,3]);
var_dump(array_keys($basic_arr));

echo "Done\n";
}

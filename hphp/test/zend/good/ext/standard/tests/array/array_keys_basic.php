<?hh
<<__EntryPoint>> function main(): void {
echo "*** Testing array_keys() on basic array operation ***\n";
$basic_arr = dict["a" => 1, "b" => 2, 2 => 2.0, -23 => "asdasd",
                    3 => vec[1,2,3]];
var_dump(array_keys($basic_arr));

echo "Done\n";
}

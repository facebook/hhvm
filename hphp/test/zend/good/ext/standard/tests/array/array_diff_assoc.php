<?hh <<__EntryPoint>> function main(): void {
$array1 = array("a" => "green", "b" => "brown", "c" => "blue", "red", "");
$array2 = array("a" => "green", "yellow", "red", TRUE);
$array3 = array("red", "a"=>"brown", "");
$result = varray[];
$result[] = array_diff_assoc($array1, $array2);
$result[] = array_diff_assoc($array1, $array3);
$result[] = array_diff_assoc($array2, $array3);
$result[] = array_diff_assoc($array1, $array2, $array3);
print_r($result);
}

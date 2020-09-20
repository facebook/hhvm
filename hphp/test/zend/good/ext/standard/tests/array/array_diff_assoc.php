<?hh <<__EntryPoint>> function main(): void {
$array1 = darray["a" => "green", "b" => "brown", "c" => "blue", 0 => "red", 1 => ""];
$array2 = darray["a" => "green", 0 => "yellow", 1 => "red", 2 => TRUE];
$array3 = darray[0 => "red", "a" => "brown", 1 => ""];
$result = varray[];
$result[] = array_diff_assoc($array1, $array2);
$result[] = array_diff_assoc($array1, $array3);
$result[] = array_diff_assoc($array2, $array3);
$result[] = array_diff_assoc($array1, $array2, $array3);
print_r($result);
}

<?hh <<__EntryPoint>> function main(): void {
$array1 = dict["a" => "green", "b" => "brown", "c" => "blue", 0 => "red", 1 => ""];
$array2 = dict["a" => "green", 0 => "yellow", 1 => "red", 2 => TRUE];
$array3 = dict[0 => "red", "a"=>"brown", 1 => ""];
$result = vec[];
$result[] = array_diff_key($array1, $array2);
$result[] = array_diff_key($array1, $array3);
$result[] = array_diff_key($array2, $array3);
$result[] = array_diff_key($array1, $array2, $array3);

var_dump($result);
}

<?hh <<__EntryPoint>> function main(): void {
$array = dict[1 => 1, 2 => 2, 3 => 3];
var_dump ($array);
error_reporting(0);
for ($i = 0; $i < (sizeof($array) + 1); $i++) {
    echo "[$i]\n";
    var_dump (array_chunk ($array, $i));
    var_dump (array_chunk ($array, $i, TRUE));
    var_dump (array_chunk ($array, $i, FALSE));
    echo "\n";
}
error_reporting(E_ALL);
}

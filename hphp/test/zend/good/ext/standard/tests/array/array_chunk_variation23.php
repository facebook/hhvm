<?hh <<__EntryPoint>> function main(): void {
$array = dict["p" => 1, "q" => 2, "r" => 3, "s" => 4, "u" => 5, "v" => 6];
var_dump ($array);
for ($i = 0; $i < (sizeof($array) + 1); $i++) {
    echo "[$i]\n";
    var_dump (@array_chunk ($array, $i));
    var_dump (@array_chunk ($array, $i, TRUE));
    var_dump (@array_chunk ($array, $i, FALSE));
    echo "\n";
}
}

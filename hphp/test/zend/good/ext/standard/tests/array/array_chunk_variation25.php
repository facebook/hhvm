<?hh <<__EntryPoint>> function main(): void {
$array = darray ["p" => "A", "q" => "B", "r" => "C", "s" => "D", "u" => "E", "v" => "F"];
var_dump ($array);
for ($i = 0; $i < (sizeof($array) + 1); $i++) {
    echo "[$i]\n";
    var_dump (@array_chunk ($array, $i));
    var_dump (@array_chunk ($array, $i, TRUE));
    var_dump (@array_chunk ($array, $i, FALSE));
    echo "\n";
}
}

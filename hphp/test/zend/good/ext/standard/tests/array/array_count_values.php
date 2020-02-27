<?hh <<__EntryPoint>> function main(): void {
$arrays = varray [
    varray [],
    varray [0],
    varray [1],
    varray [-1],
    varray [0, 0],
    varray [0, 1],
    varray [1, 1],
    varray [1, "hello", 1, "world", "hello"],
    varray ["hello", "world", "hello"],
    varray ["", "world", "", "hello", "world", "hello", "hello", "world", "hello"],
    varray [0, varray [1, "hello", 1, "world", "hello"]],
    varray [1, varray [1, "hello", 1, "world", "hello"], varray [1, "hello", 1, "world", "hello"], varray [1, "hello", 1, "world", "hello"]],
];

foreach ($arrays as $item) {
    var_dump (@array_count_values ($item));
    echo "\n";
}
}

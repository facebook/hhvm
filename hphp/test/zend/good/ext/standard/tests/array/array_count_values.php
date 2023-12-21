<?hh <<__EntryPoint>> function main(): void {
$arrays = vec[
    vec[],
    vec[0],
    vec[1],
    vec[-1],
    vec[0, 0],
    vec[0, 1],
    vec[1, 1],
    vec[1, "hello", 1, "world", "hello"],
    vec["hello", "world", "hello"],
    vec["", "world", "", "hello", "world", "hello", "hello", "world", "hello"],
    vec[0, vec[1, "hello", 1, "world", "hello"]],
    vec[1, vec[1, "hello", 1, "world", "hello"], vec[1, "hello", 1, "world", "hello"], vec[1, "hello", 1, "world", "hello"]],
];

foreach ($arrays as $item) {
    var_dump (@array_count_values ($item));
    echo "\n";
}
}

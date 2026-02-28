<?hh <<__EntryPoint>> function main(): void {
$values = vec[
    vec["01-23-45-67-89-ab", null],
    vec["01-23-45-67-89-ab", dict["options" => dict["separator" => "-"]]],
    vec["01-23-45-67-89-ab", dict["options" => dict["separator" => "."]]],
    vec["01-23-45-67-89-ab", dict["options" => dict["separator" => ":"]]],
    vec["01-23-45-67-89-AB", null],
    vec["01-23-45-67-89-aB", null],
    vec["01:23:45:67:89:ab", null],
    vec["01:23:45:67:89:AB", null],
    vec["01:23:45:67:89:aB", null],
    vec["01:23:45-67:89:aB", null],
    vec["xx:23:45:67:89:aB", null],
    vec["0123.4567.89ab", null],
    vec["01-23-45-67-89-ab", dict["options" => dict["separator" => "--"]]],
    vec["01-23-45-67-89-ab", dict["options" => dict["separator" => ""]]],
];
foreach ($values as $value) {
    var_dump(filter_var($value[0], FILTER_VALIDATE_MAC, $value[1]));
}

echo "Done\n";
}

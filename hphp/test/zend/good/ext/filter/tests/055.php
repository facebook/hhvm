<?hh <<__EntryPoint>> function main(): void {
$values = varray[
    varray["01-23-45-67-89-ab", null],
    varray["01-23-45-67-89-ab", darray["options" => darray["separator" => "-"]]],
    varray["01-23-45-67-89-ab", darray["options" => darray["separator" => "."]]],
    varray["01-23-45-67-89-ab", darray["options" => darray["separator" => ":"]]],
    varray["01-23-45-67-89-AB", null],
    varray["01-23-45-67-89-aB", null],
    varray["01:23:45:67:89:ab", null],
    varray["01:23:45:67:89:AB", null],
    varray["01:23:45:67:89:aB", null],
    varray["01:23:45-67:89:aB", null],
    varray["xx:23:45:67:89:aB", null],
    varray["0123.4567.89ab", null],
    varray["01-23-45-67-89-ab", darray["options" => darray["separator" => "--"]]],
    varray["01-23-45-67-89-ab", darray["options" => darray["separator" => ""]]],
];
foreach ($values as $value) {
    var_dump(filter_var($value[0], FILTER_VALIDATE_MAC, $value[1]));
}

echo "Done\n";
}

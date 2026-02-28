<?hh <<__EntryPoint>> function main(): void {
$test = dict[
    '0'                            => 0.0,
    '12345678900.1234567165'    => 12345678900.1234567165,
    '1,234,567,890.1234567165'    => 1234567890.1234567165,
    '-1,234,567,890.1234567165'    => -1234567890.1234567165,
    '1234,567,890.1234567165'    => false,
    '1,234,567,89.1234567165'    => false,
    '1,234,567,8900.1234567165'    => false,
    '1.234.567.890.1234567165'    => false,
    '1,234,567,8900.123,456'    => false,
];
foreach ($test as $src => $dst) {
    $out = filter_var($src, FILTER_VALIDATE_FLOAT, dict["flags"=>FILTER_FLAG_ALLOW_THOUSAND]);
    if ($dst !== $out) {
        if ($out === false) {
            echo "$src -> false != $dst\n";
        } else if ($dst === false) {
            echo "$src -> $out != false\n";
        } else {
            echo "$src -> $out != $dst\n";
        }
    }
}

echo "Ok\n";
}

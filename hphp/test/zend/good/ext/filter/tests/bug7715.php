<?hh <<__EntryPoint>> function main(): void {
$data = vec[
    '.23',
    '-42',
    '+42',
    '.4',
    '-.4',
    '1000000000000',
    '-1000000000000',
    '02.324'
];
foreach ($data as $val) {
    $res = filter_var($val, FILTER_VALIDATE_FLOAT);
    var_dump($res);
}
echo "\n";
}

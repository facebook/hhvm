<?hh <<__EntryPoint>> function main(): void {
$data = vec[
        'E2',
        '10E',
        '2E-',
        'E-2',
        '+E2'
        ];
$out = filter_var($data, FILTER_VALIDATE_FLOAT, FILTER_REQUIRE_ARRAY);
var_dump($out);
}

<?hh <<__EntryPoint>> function main(): void {
$tests = vec[
    'foo',
    vec[],
    vec[0],
    vec[3],
    vec[3, 3],
    vec[0.5, 2],
    vec[99999999, 99999999],
    vec[8.993, 7443241,988, HH\Lib\Legacy_FIXME\cast_for_arithmetic(sprintf("%u", -1))+0.44],
    vec[2,sprintf("%u", -1)],
];

foreach ($tests as $v) {
    var_dump(array_product($v));
}
}

<?hh <<__EntryPoint>> function main(): void {
$tests = varray[
    'foo',
    varray[],
    varray[0],
    varray[3],
    varray[3, 3],
    varray[0.5, 2],
    varray[99999999, 99999999],
    varray[8.993, 7443241,988, sprintf("%u", -1)+0.44],
    varray[2,sprintf("%u", -1)],
];

foreach ($tests as $v) {
    var_dump(array_product($v));
}
}

<?hh
<<__EntryPoint>> function main(): void {
foreach(vec[vec[1,2], vec[3,4]] as list($a, $b)) {
    var_dump($a . $b);
}

$array = vec[
    vec['a', 'b'],
    vec['c', 'd'],
];

foreach ($array as list($a, $b)) {
    var_dump($a . $b);
}


$multi = vec[
    vec[vec[1,2], vec[3,4]],
    vec[vec[5,6], vec[7,8]],
];

foreach ($multi as list(list($a, $b), list($c, $d))) {
    var_dump($a . $b . $c . $d);
}

foreach ($multi as $key => list(list($a, $b), list($c, $d))) {
    var_dump($key . $a . $b . $c . $d);
}
}

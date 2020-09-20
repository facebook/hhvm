<?hh
<<__EntryPoint>> function main(): void {
foreach(varray[varray[1,2], varray[3,4]] as list($a, $b)) {
    var_dump($a . $b);
}

$array = varray[
    varray['a', 'b'],
    varray['c', 'd'],
];

foreach ($array as list($a, $b)) {
    var_dump($a . $b);
}


$multi = varray[
    varray[varray[1,2], varray[3,4]],
    varray[varray[5,6], varray[7,8]],
];

foreach ($multi as list(list($a, $b), list($c, $d))) {
    var_dump($a . $b . $c . $d);
}

foreach ($multi as $key => list(list($a, $b), list($c, $d))) {
    var_dump($key . $a . $b . $c . $d);
}
}

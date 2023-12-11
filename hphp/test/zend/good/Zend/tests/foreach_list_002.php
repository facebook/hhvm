<?hh
<<__EntryPoint>> function main(): void {
foreach (vec[vec[1,2], vec[3,4]] as list($a, )) {
    var_dump($a);
}

$array = vec[vec['a', 'b'], 'c', 'd'];

foreach($array as list(list(), $a)) {
   var_dump($a);
}
}

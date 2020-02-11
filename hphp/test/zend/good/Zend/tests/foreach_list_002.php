<?hh
<<__EntryPoint>> function main(): void {
foreach (varray[varray[1,2], varray[3,4]] as list($a, )) {
    var_dump($a);
}

$array = varray[varray['a', 'b'], 'c', 'd'];

foreach($array as list(list(), $a)) {
   var_dump($a);
}
}

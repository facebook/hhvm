<?hh <<__EntryPoint>> function main(): void {
$a = varray[varray['A'], varray['B'], varray['C'], varray['D']];
$b = varray[$a, $a, $a, $a];
$c = varray[$b, $b, $b, $b];
$d = varray[$c, $c, $c, $c];
$e = varray[$d, $d, $d, $d];
var_dump(json_encode($e, 0, 3));
var_dump(json_encode($e, 0, 4));
var_dump(json_encode($e, 0, 5));
var_dump(json_encode($e, 0, 6));
var_dump(json_encode($e, 0, 7));
var_dump(json_encode($e));
$E = json_decode(json_encode($e));
var_dump($E == $e);
}

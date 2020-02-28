<?hh
<<__EntryPoint>> function main(): void {
$a = darray[];
$a[0] = varray[$a];
var_dump($a);
$b = varray[varray[$b]];
var_dump($b);
$c = varray[$c];
var_dump($c);
}

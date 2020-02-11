<?hh <<__EntryPoint>> function main(): void {
$f = varray[varray['A', 'A']];

$z = substr_replace($f, $f, $f, 1);
var_dump($z, $f);
}

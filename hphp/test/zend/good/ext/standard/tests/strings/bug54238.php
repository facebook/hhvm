<?hh <<__EntryPoint>> function main(): void {
$f = array(array('A', 'A'));

$z = substr_replace($f, $f, $f, 1);
var_dump($z, $f);
}

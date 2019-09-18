<?hh
class AO extends ArrayObject {
}
<<__EntryPoint>> function main(): void {
$o = new AO();
$o['plop'] = $o;

var_dump($o);
}

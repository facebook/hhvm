<?hh
class cls {
}
<<__EntryPoint>> function main(): void {
$obj = new cls;
$a = array(1,2);
unset($a[$obj]);
var_dump($a);
}

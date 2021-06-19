<?hh
<<__EntryPoint>> function main(): void {
$a = null;
$a += new Exception();
var_dump($a);
}

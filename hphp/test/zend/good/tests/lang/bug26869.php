<?hh
const A = "1";
<<__EntryPoint>> function main(): void {
$a=dict[A => 1];
var_dump($a);
var_dump(isset($a[A]));
}

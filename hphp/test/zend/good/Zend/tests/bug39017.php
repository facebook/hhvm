<?hh
class A {}
<<__EntryPoint>> function main(): void {
foreach(($a=(object)new A()) as $v);
var_dump($a); // UNKNOWN:0
}

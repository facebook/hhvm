<?hh
<<__EntryPoint>> function main(): void {
$a=vec["a", "b", "c"];
$v=dict[];
foreach($a as $v[0]) {
	var_dump($v);
}
var_dump($a);
var_dump($v);

echo "\n";
$a=vec["a", "b", "c"];
$v=dict[];
foreach($a as $k=>$v[0]) {
	var_dump($k, $v);
}
var_dump($a);
var_dump($k, $v);
}

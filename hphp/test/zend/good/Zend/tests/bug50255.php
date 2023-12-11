<?hh
<<__EntryPoint>> function main(): void {
$arr = dict['foo' => 'bar'];

print "isset\n";
var_dump(isset($arr->foo));
var_dump(isset($arr->bar));
var_dump(isset($arr['foo']));
var_dump(isset($arr['bar']));
}

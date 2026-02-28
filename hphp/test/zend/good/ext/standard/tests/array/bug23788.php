<?hh <<__EntryPoint>> function main(): void {
$numeric = 123;
$bool = true;
$foo = vec[$numeric, $bool];
var_dump($foo);
str_replace("abc", "def", $foo);
var_dump($foo);
}

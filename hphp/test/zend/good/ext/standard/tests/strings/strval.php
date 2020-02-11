<?hh
const FOO = 'BAR';
<<__EntryPoint>> function main(): void {
$foo = 'bar';
var_dump(strval($foo));
var_dump(strval(FOO));
var_dump(strval('foobar'));
var_dump(strval(1));
var_dump(strval(1.1));
var_dump(strval(true));
var_dump(strval(false));
var_dump(strval(varray['foo']));
}

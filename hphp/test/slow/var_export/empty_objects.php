<?hh
class Foo {
}

<<__EntryPoint>>
function main_empty_objects() :mixed{
$x = new stdClass();
var_dump(var_export($x, true));
$x->herp = 'derp';
var_dump(var_export($x, true));
$y = new Foo();
var_dump(var_export($y, true));
}

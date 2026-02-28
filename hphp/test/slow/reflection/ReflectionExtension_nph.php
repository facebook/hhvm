<?hh

// Test native property handling with ReflectionExtension

<<__EntryPoint>>
function main_reflection_extension_nph() :mixed{
$x = new ReflectionExtension("mysql");
var_dump($x->getName());
var_dump($x->name);
// This should fatal since $name is technically a read only prop
$x->name = "Bad";
// We should never get here
var_dump($x->name);
}

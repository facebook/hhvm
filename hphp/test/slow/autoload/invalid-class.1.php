<?hh

<<__EntryPoint>>
function main_invalid_class_1() {
$name = '-illegal-class';

var_dump(class_exists($name));
new $name();
}

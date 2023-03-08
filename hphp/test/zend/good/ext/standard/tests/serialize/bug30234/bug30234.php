<?hh

<<__EntryPoint>> function main(): void {
var_dump(interface_exists('autoload_interface', false));
var_dump(class_exists('autoload_implements', false));

$o = unserialize('O:19:"autoload_implements":0:{}');

var_dump($o);
var_dump($o is autoload_interface);
unset($o);

var_dump(interface_exists('autoload_interface', false));
var_dump(class_exists('autoload_implements', false));

echo "===DONE===\n";
}

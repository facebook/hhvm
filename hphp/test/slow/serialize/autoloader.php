<?hh

<<__EntryPoint>>
function main_autoloader() {
$serialized_str = 'O:1:"A":0:{}';

var_dump(unserialize($serialized_str, darray['allowed_classes' => false]));
var_dump(unserialize($serialized_str));
}

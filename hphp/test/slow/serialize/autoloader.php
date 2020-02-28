<?hh

function __random_autoloader($class) {
  var_dump($class);
}

<<__EntryPoint>>
function main_autoloader() {
spl_autoload_register('__random_autoloader');

$serialized_str = 'O:1:"A":0:{}';

var_dump(unserialize($serialized_str, darray['allowed_classes' => false]));
var_dump(unserialize($serialized_str));
}

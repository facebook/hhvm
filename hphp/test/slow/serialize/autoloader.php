<?hh

<<__EntryPoint>>
function main_autoloader() :mixed{
$serialized_str = 'O:1:"A":0:{}';

var_dump(unserialize($serialized_str, dict['allowed_classes' => false]));
var_dump(unserialize($serialized_str));
}

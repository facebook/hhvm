<?hh


function __autoload($name) {
  echo "autoload $name\n";
}


<<__EntryPoint>>
function main_get_class_vars() {
var_dump(get_class_vars('nope'));
}

<?hh

function __autoload($class) {
  echo "exiting\n";
  exit(1);
}

function get_instance($name) {
  $classname = "X$name";
  return new $classname;
}


<<__EntryPoint>>
function main_autoload_exit() {
get_instance('test');
}

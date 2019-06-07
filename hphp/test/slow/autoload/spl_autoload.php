<?hh

function my_autoload($class) {
  return class_exists($class, true);
}


<<__EntryPoint>>
function main_spl_autoload() {
spl_autoload_register('my_autoload');

$a = new A();
}

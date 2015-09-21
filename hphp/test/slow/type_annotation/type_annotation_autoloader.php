<?hh

class C {
  const type T = this;
}

function my_autoloader($class) {
  echo $class."\n";
}

spl_autoload_register('my_autoloader');

var_dump(type_structure(C::class, 'T'));

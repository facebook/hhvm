<?hh

<<__EntryPoint>>
function main_1232() {
  spl_autoload_register(function($f) {
    var_dump(1);
  });
  spl_autoload_register(function($f) {
    var_dump(2);
  });
  class_exists('A');
}

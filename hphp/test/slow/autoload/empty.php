<?hh


<<__EntryPoint>>
function main_empty() {
spl_autoload_register(function($class) {
  var_dump($class);
});

var_dump(method_exists('', 'foo'));
}

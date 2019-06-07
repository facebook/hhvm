<?hh

function __autoload($name) {
  if ($name == 'CaT') {
    include '1228.inc';
  }
  var_dump($name);
}

<<__EntryPoint>>
function main_1228() {
new CaT(1);
class_exists('cat', false);
}

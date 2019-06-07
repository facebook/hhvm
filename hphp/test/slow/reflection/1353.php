<?hh

function __autoload($name) {
  switch ($name) {
    case 'C': include '1353-1.inc'; break;
    case 'M': include '1353-2.inc'; break;
    default:  include '1353-3.inc'; break;
  }
  var_dump($name);
}

<<__EntryPoint>>
function main_1353() {
$r1 = new ReflectionClass('C');
$r2 = new ReflectionMethod('M', 'foo');
}

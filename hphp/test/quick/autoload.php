<?hh
function __autoload($cls) {
  echo "__autoload $cls\n";
  if ($cls === 'C') {
    include 'autoload1.inc';
  }
}
function my_autoload_func($cls) {
  echo "my_autoload_func $cls\n";
  if ($cls === 'D') {
    include 'autoload2.inc';
  }
}

var_dump(is_callable(varray['D', 'foo']));
spl_autoload_register('my_autoload_func');
var_dump(is_callable(varray['D', 'foo']));

var_dump(is_callable(varray['C', 'foo']));
spl_autoload_register('__autoload');
var_dump(is_callable(varray['C', 'foo']));


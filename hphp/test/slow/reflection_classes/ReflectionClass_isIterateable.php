<?hh

class IteratorClass implements Iterator {
  public function __construct() { }
  public function key() { }
  public function current() { }
  function next() { }
  function valid() { }
  function rewind() { }
}
class DerivedClass extends IteratorClass {}
class NonIterator {}

function dump_iterateable($class) {
  $reflection = new ReflectionClass($class);
  var_dump($reflection->isIterateable());
}


<<__EntryPoint>>
function main_reflection_class_is_iterateable() {
$classes = varray["IteratorClass", "DerivedClass", "NonIterator"];

foreach ($classes as $class) {
  echo "Is $class iterateable? ";
  dump_iterateable($class);
}
}

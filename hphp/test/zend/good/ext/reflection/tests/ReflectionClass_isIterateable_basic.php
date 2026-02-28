<?hh

class IteratorClass implements Iterator {
    public function __construct() { }
    public function key() :mixed{}
    public function current() :mixed{}
    function next()    :mixed{}
    function valid() :mixed{}
    function rewind() :mixed{}
}
class DerivedClass extends IteratorClass {}
class NonIterator {}

function dump_iterateable($class) :mixed{
    $reflection = new ReflectionClass($class);
    var_dump($reflection->isIterateable());
}
<<__EntryPoint>> function main(): void {
$classes = vec["IteratorClass", "DerivedClass", "NonIterator"];
foreach ($classes as $class) {
    echo "Is $class iterateable? ";
    dump_iterateable($class);
}
}

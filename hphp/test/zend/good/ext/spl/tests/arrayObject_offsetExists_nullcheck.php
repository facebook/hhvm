<?hh <<__EntryPoint>> function main() {
$ao = new ArrayObject(array('foo' => null));
var_dump($ao->offsetExists('foo'));
}

<?hh <<__EntryPoint>> function main(): void {
$ao = new ArrayObject(array('foo' => null));
var_dump($ao->offsetExists('foo'));
}

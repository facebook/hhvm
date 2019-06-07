<?hh

class fOo {
}
interface ioO {
}

<<__EntryPoint>>
function main_1358() {
$c = new ReflectionClass('Foo');
$i = new ReflectionClass('Ioo');
var_dump($c->getFileName() !== '');
var_dump($i->getFileName() !== '');
}

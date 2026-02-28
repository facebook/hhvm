<?hh

class Foo {
}
interface Ioo {
}

<<__EntryPoint>>
function main_1358() :mixed{
$c = new ReflectionClass('Foo');
$i = new ReflectionClass('Ioo');
var_dump($c->getFileName() !== '');
var_dump($i->getFileName() !== '');
}

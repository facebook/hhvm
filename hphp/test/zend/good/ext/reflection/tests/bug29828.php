<?hh

interface Bla
{
    function bla();
}

class BlaMore implements Bla
{
    function bla()
    {
        echo "Hello\n";
    }
}
<<__EntryPoint>> function main(): void {
$r = new ReflectionClass('BlaMore');

var_dump(count($r->getMethods()));
var_dump($r->getMethod('bla')->isConstructor());
var_dump($r->getMethod('bla')->isAbstract());

$o=new BlaMore;
$o->bla();

echo "===DONE===\n";
}

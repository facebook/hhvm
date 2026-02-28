<?hh

class TestClass
{
    /** const comment */
    const C = 0;

    function x() :mixed{}
}

class TestClass2
{
    /** const comment */
    const C = 0;

    public $x;
}

<<__EntryPoint>> function main(): void {
$rm = new ReflectionMethod('TestClass', 'x');
var_dump($rm->getDocComment());

$rp = new ReflectionProperty('TestClass2', 'x');
var_dump($rp->getDocComment());
}

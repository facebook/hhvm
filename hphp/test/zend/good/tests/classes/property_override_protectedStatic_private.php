<?hh
class A
{
    protected static $p = "A::p (static)";
    static function showA()
:mixed    {
        echo self::$p . "\n";
    }
}

class B extends A
{
    private $p = "B::p";
    function showB()
:mixed    {
        echo $this->p . "\n";
    }
}

<<__EntryPoint>> function main(): void {
A::showA();

$b = new B;
$b->showA();
$b->showB();
}

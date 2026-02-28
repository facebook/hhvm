<?hh
class A
{
    private static $p = "A::p (static)";
    static function showA()
:mixed    {
        echo self::$p . "\n";
    }
}

class B extends A
{
    public $p = "B::p";
    function showB()
:mixed    {
        echo $this->p . "\n";
    }
}

<<__EntryPoint>> function main(): void {
A::showA();

$b = new B;
B::showA();
$b->showB();
}

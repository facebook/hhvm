<?hh
class A
{
    private static $p = "A::p (static)";
    static function showA()
    {
        echo self::$p . "\n";
    }
}

class B extends A
{
    protected $p = "B::p";
    function showB()
    {
        echo $this->p . "\n";
    }
}

<<__EntryPoint>> function main(): void {
A::showA();

$b = new B;
B::showA();
$b->showB();
}

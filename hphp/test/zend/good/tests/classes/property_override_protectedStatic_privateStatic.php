<?hh
class A
{
    protected static $p = "A::p (static)";
    static function showA()
    {
        echo self::$p . "\n";
    }
}

class B extends A
{
    private static $p = "B::p (static)";
    static function showB()
    {
        echo self::$p . "\n";
    }
}

<<__EntryPoint>> function main(): void {
A::showA();

B::showA();
B::showB();
}

<?hh

class Test
{
    static function f1(array $ar)
    {
        echo __METHOD__ . "()\n";
        var_dump($ar);
    }

    static function f2(array $ar = NULL)
    {
        echo __METHOD__ . "()\n";
        var_dump($ar);
    }

    static function f3(array $ar = varray[])
    {
        echo __METHOD__ . "()\n";
        var_dump($ar);
    }

    static function f4(array $ar = varray[25])
    {
        echo __METHOD__ . "()\n";
        var_dump($ar);
    }
}
<<__EntryPoint>> function main(): void {
Test::f1(varray[42]);
Test::f2(NULL);
Test::f2();
Test::f3();
Test::f4();
Test::f1(1);
}

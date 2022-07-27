<?hh

class Test
{
    static function f1(varray $ar)
    {
        echo __METHOD__ . "()\n";
        var_dump($ar);
    }

    static function f2(?varray $ar = NULL)
    {
        echo __METHOD__ . "()\n";
        var_dump($ar);
    }

    static function f3(varray $ar = varray[])
    {
        echo __METHOD__ . "()\n";
        var_dump($ar);
    }

    static function f4(varray $ar = varray[25])
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

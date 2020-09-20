<?hh

class MyObject
{
    function fail()
    {
        throw new Exception();
    }

    function __construct()
    {
        self::fail();
        echo __METHOD__ . "() Must not be reached\n";
    }

    static function test()
    {
        try
        {
            new MyObject();
        }
        catch(Exception $e)
        {
            echo "Caught\n";
        }
    }
}
<<__EntryPoint>> function main(): void {
MyObject::test();

echo "===DONE===\n";
}

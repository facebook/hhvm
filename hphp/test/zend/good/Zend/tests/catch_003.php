<?hh

class MyObject
{
    function fail()
:mixed    {
        throw new Exception();
    }

    function __construct()
    {
        self::fail();
        echo __METHOD__ . "() Must not be reached\n";
    }
}
<<__EntryPoint>> function main(): void {
try
{
    new MyObject();
}
catch(Exception $e)
{
    echo "Caught\n";
}

echo "===DONE===\n";
}

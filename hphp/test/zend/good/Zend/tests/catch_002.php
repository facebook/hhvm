<?hh

class MyObject
{
    function __construct()
    {
        throw new Exception();
        echo __METHOD__ . "() Must not be reached\n";
    }
}
<<__EntryPoint>> function main() {
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

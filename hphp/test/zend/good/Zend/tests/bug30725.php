<?hh

class Test implements IteratorAggregate
{
    function getIterator()
:mixed    {
        throw new Exception();
    }
}
<<__EntryPoint>> function main(): void {
try
{
    $it = new Test;
    foreach($it as $v)
    {
        echo "Fail\n";
    }
    echo "Wrong\n";
}
catch(Exception $e)
{
    echo "Caught\n";
}

echo "===DONE===\n";
}

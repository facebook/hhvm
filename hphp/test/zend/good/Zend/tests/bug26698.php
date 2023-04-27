<?hh

class MyObject
{
    function getNone()
    {
        throw new Exception('NONE');
    }
}

class Proxy
{
    function three($a, $b, $c)
    {
    }

    function callOne()
    {
        try
        {
            $res = new MyObject();
            $this->three($res->getNone());
        }
        catch(Exception $e)
        {
            echo 'Caught: '.$e->getMessage()."\n";
        }
    }

    function callTwo()
    {
        try
        {
            $res = new MyObject();
            $this->three(1, $res->getNone());
        }
        catch(Exception $e)
        {
            echo 'Caught: '.$e->getMessage()."\n";
        }
    }

    function callThree()
    {
        try
        {
            $res = new MyObject();
            $this->three(1, 2, $res->getNone());
        }
        catch(Exception $e)
        {
            echo 'Caught: '.$e->getMessage()."\n";
        }
    }
}
<<__EntryPoint>> function main(): void {
ini_set("report_memleaks", 0);  // the exception thrown in this test results in a memory leak, which is fine

$p = new Proxy();

$p->callOne();
$p->callTwo();
$p->callThree();
echo "===DONE===\n";
}

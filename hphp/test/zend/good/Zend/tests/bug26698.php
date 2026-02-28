<?hh

class MyObject
{
    function getNone()
:mixed    {
        throw new Exception('NONE');
    }
}

class Proxy
{
    function three($a, $b, $c)
:mixed    {
    }

    function callOne()
:mixed    {
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
:mixed    {
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
:mixed    {
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

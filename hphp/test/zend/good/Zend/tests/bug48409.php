<?hh

class ABCException extends Exception {}

class BBB
{
    public function xyz($d, $x)
:mixed    {
        if ($x == 34) {
            throw new ABCException;
        }
        return dict['foo' => 'xyz'];
    }
}

class CCC
{
    public function process($p)
:mixed    {
        return $p;
    }
}

class AAA
{
    public function func()
:mixed    {
        $b = new BBB;
        $c = new CCC;
        $i = 34;
        $item = dict['foo' => 'bar'];
        try {
            $c->process($b->xyz($item['foo'], $i));
        }
        catch(ABCException $e) {
            $b->xyz($item['foo'], $i);
        }
    } // end func();
}

class Runner
{
    public function run($x)
:mixed    {
        try {
            $x->func();
        }
        catch(ABCException $e) {
            throw new Exception;
        }
    }
}
<<__EntryPoint>> function main(): void {
try {
    $runner = new Runner;
    $runner->run(new AAA);
}
catch(Exception $e) {
    die('Exception thrown');
}
}

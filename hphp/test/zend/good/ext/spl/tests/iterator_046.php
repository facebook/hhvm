<?hh

class MyFoo
{
    function __toString()
    {
        return 'foo';
    }
}

class MyCachingIterator extends CachingIterator
{
    function __construct(Iterator $it, $flags = 0)
    {
        parent::__construct($it, $flags);
    }

    function fill()
    {
        echo __METHOD__ . "()\n";
        foreach($this as $v) ;
    }

    function show()
    {
        echo __METHOD__ . "()\n";
        foreach($this as $v)
        {
            var_dump((string)$this);
        }
    }
}
<<__EntryPoint>> function main(): void {
$it = new MyCachingIterator(new ArrayIterator(darray[0 => 0, 'foo' => 1, 'bar' => 2]), CachingIterator::TOSTRING_USE_KEY);

$it->fill();
$it->show();

echo "===DONE===\n";
}

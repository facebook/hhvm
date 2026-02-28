<?hh

class ArrayIteratorEx extends ArrayIterator
{
    function rewind()
:mixed    {
        echo __METHOD__ . "\n";
        ArrayIterator::rewind();
    }

    function valid()
:mixed    {
        echo __METHOD__ . "\n";
        return ArrayIterator::valid();
    }

    function key()
:mixed    {
        echo __METHOD__ . "\n";
        return ArrayIterator::key();
    }

    function current()
:mixed    {
        echo __METHOD__ . "\n";
        return ArrayIterator::current();
    }

    function next()
:mixed    {
        echo __METHOD__ . "\n";
        return ArrayIterator::next();
    }
}
<<__EntryPoint>> function main(): void {
$ar = new ArrayIteratorEx(vec[1,2]);
foreach($ar as $k => $v)
{
    var_dump($k);
    var_dump($v);
}

echo "===DONE===\n";
}

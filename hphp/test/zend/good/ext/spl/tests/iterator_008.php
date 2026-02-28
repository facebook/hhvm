<?hh

class ArrayIteratorEx extends ArrayIterator
{
    function rewind()
:mixed    {
        echo __METHOD__ . "\n";
        parent::rewind();
    }
    function valid()
:mixed    {
        echo __METHOD__ . "\n";
        return parent::valid();
    }
    function current()
:mixed    {
        echo __METHOD__ . "\n";
        return parent::current();
    }
    function key()
:mixed    {
        echo __METHOD__ . "\n";
        return parent::key();
    }
    function next()
:mixed    {
        echo __METHOD__ . "\n";
        parent::next();
    }
}
<<__EntryPoint>> function main(): void {
$it = new InfiniteIterator(new ArrayIteratorEx(range(0,2)));

$pos =0;

foreach ($it as $v) {
    var_dump($v);
    if ($pos++ > 5) {
        break;
    }
}

echo "===DONE===\n";
}

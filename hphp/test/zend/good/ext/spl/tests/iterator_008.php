<?hh

class ArrayIteratorEx extends ArrayIterator
{
    function rewind()
    {
        echo __METHOD__ . "\n";
        parent::rewind();
    }
    function valid()
    {
        echo __METHOD__ . "\n";
        return parent::valid();
    }
    function current()
    {
        echo __METHOD__ . "\n";
        return parent::current();
    }
    function key()
    {
        echo __METHOD__ . "\n";
        return parent::key();
    }
    function next()
    {
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

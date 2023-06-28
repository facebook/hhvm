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

class NoRewindIteratorEx extends NoRewindIterator
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
$it = new NoRewindIteratorEx(new ArrayIteratorEx(range(0,3)));

echo "===0===\n";
foreach ($it->getInnerIterator() as $v) {
    var_dump($v);
}

echo "===1===\n";
foreach ($it as $v) {
    var_dump($v);
}

$pos =0;

$it = new NoRewindIteratorEx(new ArrayIteratorEx(range(0,3)));

echo "===2===\n";
foreach ($it as $v) {
    var_dump($v);
    if ($pos++ > 1) {
        break;
    }
}

echo "===3===\n";
foreach ($it as $v) {
    var_dump($v);
}

echo "===4===\n";
foreach ($it as $v) {
    var_dump($v);
}
echo "===DONE===\n";
}

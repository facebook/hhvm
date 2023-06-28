<?hh

class EmptyIteratorEx extends EmptyIterator
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
foreach (new EmptyIteratorEx() as $v) {
    var_dump($v);
}

echo "===DONE===\n";
}

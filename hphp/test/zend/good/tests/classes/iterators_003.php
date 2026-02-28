<?hh
class c_iter implements Iterator {

    private $obj;
    private $num = 0;

    function __construct($obj) {
        echo __METHOD__ . "\n";
        $this->obj = $obj;
    }
    function rewind() :mixed{
        echo __METHOD__ . "\n";
    }
    function valid() :mixed{
        $more = $this->num < $this->obj->max;
        echo __METHOD__ . ' = ' .($more ? 'true' : 'false') . "\n";
        return $more;
    }
    function current() :mixed{
        echo __METHOD__ . "\n";
        return $this->num;
    }
    function next() :mixed{
        echo __METHOD__ . "\n";
        $this->num++;
    }
    function key() :mixed{
        return $this->num;
    }
}

class c implements IteratorAggregate {

    public $max = 4;

    function getIterator() :mixed{
        echo __METHOD__ . "\n";
        return new c_iter($this);
    }
}
<<__EntryPoint>> function main(): void {
$t = new c();

foreach($t as $v) {
    if ($v == 0) {
        echo "continue outer\n";
        continue;
    }
    foreach($t as $w) {
        if ($w == 1) {
            echo "continue inner\n";
            continue;
        }
        if ($w == 2) {
            echo "break inner\n";
            break;
        }
        echo "double:$v:$w\n";
    }
    if ($v == 2) {
        echo "break outer\n";
        break;
    }
}

print "Done\n";
}

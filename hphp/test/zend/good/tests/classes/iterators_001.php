<?hh
class c_iter implements Iterator {

    private $obj;
    private $num = 0;

    function __construct($obj) {
        echo __METHOD__ . "\n";
        $this->num = 0;
        $this->obj = $obj;
    }
    function rewind() :mixed{
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
        echo __METHOD__ . "\n";
        switch($this->num) {
            case 0: return "1st";
            case 1: return "2nd";
            case 2: return "3rd";
            default: return "???";
        }
    }
}

class c implements IteratorAggregate {

    public $max = 3;

    function getIterator() :mixed{
        echo __METHOD__ . "\n";
        return new c_iter($this);
    }
}
<<__EntryPoint>> function main(): void {
echo "===Array===\n";

$a = vec[0,1,2];
foreach($a as $v) {
    echo "array:$v\n";
}

echo "===Manual===\n";
$t = new c();
for ($iter = $t->getIterator(); $iter->valid(); $iter->next()) {
    echo $iter->current() . "\n";
}

echo "===foreach/std===\n";
foreach($t as $v) {
    echo "object:$v\n";
}

echo "===foreach/rec===\n";
foreach($t as $v) {
    foreach($t as $w) {
        echo "double:$v:$w\n";
    }
}

echo "===foreach/key===\n";
foreach($t as $i => $v) {
    echo "object:$i=>$v\n";
}

print "Done\n";
exit(0);
}

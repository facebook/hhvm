<?hh

class c1 {}

class c2 {

    public $max = 3;
    public $num = 0;

    function current() :mixed{
        echo __METHOD__ . "\n";
        return $this->num;
    }
    function next() :mixed{
        echo __METHOD__ . "\n";
        $this->num++;
    }
    function valid() :mixed{
        echo __METHOD__ . "\n";
        return $this->num < $this->max;
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

<<__EntryPoint>> function main(): void {
echo "1st try\n";

$obj = new c1();

foreach($obj as $w) {
    echo "object:$w\n";
}

echo "2nd try\n";

$obj = new c2();

foreach($obj as $v => $w) {
    echo "object:$v=>$w\n";
}

print "Done\n";
}

<?hh

class Caller {
    public $x = varray[1, 2, 3];

    function __call($m, $a) {
        echo "Method $m called:\n";
        var_dump($a);
        return $this->x;
    }
}
<<__EntryPoint>> function main(): void {
$foo = new Caller();
$a = $foo->test(1, '2', 3.4, true);
var_dump($a);
}

<?hh

class Test {
    public $func;
    function __construct() {
        $this->func = function() {
            echo __METHOD__ . "()\n";
        };
    }
    private function func() :mixed{
        echo __METHOD__ . "()\n";
    }
}
<<__EntryPoint>> function main(): void {
$o = new Test;
$f = $o->func;
$f();
$o->func();

echo "===DONE===\n";
}

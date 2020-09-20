<?hh

class early {
    function __construct() {
        echo __CLASS__ . "::" . __FUNCTION__ . "\n";
    }
}

class late {
    function __construct() {
        echo __CLASS__ . "::" . __FUNCTION__ . "\n";
    }
}
<<__EntryPoint>> function main(): void {
$t = new early();
$t->__construct();
unset($t);
$t = new late();
//unset($t); delay to end of script

echo "Done\n";
}

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
$t = null;
$t = new late();
//$t = null; delay to end of script

echo "Done\n";
}

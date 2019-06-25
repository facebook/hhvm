<?hh

function foo( &$state ) {
    $contentDict = end( &$state );
    for ( $contentDict = end( &$state ); $contentDict !== false; $contentDict = prev( &$state ) ) {
    echo key(&$state) . " => " . current(&$state) . "\n";
    }
}
<<__EntryPoint>> function main(): void {
$state = array("one" => 1, "two" => 2, "three" => 3);
foo(&$state);
reset(&$state);
var_dump( key(&$state), current(&$state) );
}

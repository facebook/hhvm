<?hh

function foo( inout $state ) {
    $contentDict = end(inout $state );
    for ( $contentDict = end(inout $state ); $contentDict !== false; $contentDict = prev(inout $state ) ) {
    echo key($state) . " => " . current($state) . "\n";
    }
}
<<__EntryPoint>> function main(): void {
$state = darray["one" => 1, "two" => 2, "three" => 3];
foo(inout $state);
reset(inout $state);
var_dump( key($state), current($state) );
}

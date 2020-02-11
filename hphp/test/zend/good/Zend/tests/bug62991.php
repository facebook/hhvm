<?hh

function test( array $array )
{
    $closure = function() use ( $array ) {
        print_r( $array );
        yield "hi";
    };
    return $closure();
}

function test2( array $array )
{
    $closure = function() use ( $array ) {
        print_r( $array );
        yield "hi";
    };
    return $closure; // if you return the $closure and call it outside this function it works.
}
<<__EntryPoint>> function main(): void {
$generator = test(varray[ 1, 2, 3 ] );
foreach($generator as $something) {
}

$generator = test2(varray[ 1, 2, 3 ] );
foreach($generator() as $something) {
}


$generator = test2(varray[ 1, 2, 3 ] );

echo "okey\n";
}

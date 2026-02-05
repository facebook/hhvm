<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function foo(int $x, int $y, named int $a, named int $b) {
    var_dump(vec[$x, $y, $a, $b]);
}

function echo_and_return(int $x): int {
    echo (string)$x . "\n";
    return $x;
}

function increment(inout int $x): void {
    $old = $x;
    $x += 1;
    return $old;
}

<<__EntryPoint>>
function main() {
    foo(echo_and_return(1), echo_and_return(2), a=echo_and_return(3), b=echo_and_return(4));
    foo(echo_and_return(1), echo_and_return(2), b=echo_and_return(4), a=echo_and_return(3));
    foo(b=echo_and_return(4), a=echo_and_return(3), echo_and_return(1), echo_and_return(2));
    $x = 1;
    // TODO(named_params): Regardless of the inout order, the old value gets
    // used for the named args. Should probably be banned.
    foo(increment(inout $x), a=$x, increment(inout $x), b=$x);
    $x = 1;
    foo(increment(inout $x), b=$x, increment(inout $x), a=$x);
    $x = 1;
    foo(increment(inout $x), b=$x, increment(inout $x), a=increment(inout $x));
}

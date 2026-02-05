<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function foo(int $x, int $y, named int $a, named int $b) {
    var_dump($x);
    var_dump($y);
    var_dump($a);
    var_dump($b);
}

<<__EntryPoint>>
function main() {
    foo(1, 2, a=3, b=4);
    foo(1, 2, b=4, a=3);
    foo(a=3, 1, 2, b=4);
}

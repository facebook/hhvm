<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function foo(int $x = 0, int $y = 1, int $z = 2, named $a = shape('a' => 2), named $b = () ==> { return 3 + 4; }) {
    var_dump(vec[$x, $y, $z, $a, $b()]);
}


<<__EntryPoint>>
function main() {
    foo();
    foo(a=shape('a' => 1));
    foo(b=() ==> { return 42; });
    foo(84, b=() ==> { return 42; });
}

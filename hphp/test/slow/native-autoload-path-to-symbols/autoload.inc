class A {}

const int C = 1;

newtype T = string;

function func() {}

<<__Memoize>>
function func_memoized() {
    $closure = () ==> {};
    $closure();
}

<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

class C {
    public function foo(int $x, int $y, named int $a, named int $b) {
        var_dump(vec[$x, $y, $a, $b]);
    }
}

class D extends C {
    <<__Override>>
    public function foo(int $x, int $y, named int $a, named int $b) {
        var_dump(vec[$x + 100, $y + 100, $a + 100, $b +100]);
    }
}

function dispatch(C $c) {
    $c->foo(1, 2, a=3, b=4);
}

<<__EntryPoint>>
function main() {
    dispatch(new C());
    dispatch(new D());
}

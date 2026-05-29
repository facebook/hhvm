<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

class C {
    public function foo(int $x, int $y, named int $a, named int $b) {
        var_dump(vec[$x, $y, $a, $b]);
    }

    public function bar(named int $a, int ...$args) {
        $v = vec[$a];
        foreach ($args as $arg) {
            $v[] = $arg;
        }
        var_dump($v);
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
    $c->bar(a=0);
    $c->bar(a=0, 1);
    $c->bar(a=0, 1, 2);
}

<<__EntryPoint>>
function main() {
    dispatch(new C());
    dispatch(new D());
}

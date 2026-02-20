<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>


function foo(int $x, int $y, named int $a, named int $b = 4, named int $c = 5, int ...$args) {
    $v = vec[];
    $v[] = $x;
    $v[] = $y;
    $v[] = $a;
    $v[] = $b;
    $v[] = $c;
    foreach ($args as $arg) {
        $v[] = $arg;
    }
    var_dump($v);
}

function bar(int $x = 1, int $y, named int $a = 3, named int $b = 4, named int $c = 5) {
    $v = vec[];
    $v[] = $x;
    $v[] = $y;
    $v[] = $a;
    $v[] = $b;
    $v[] = $c;
    var_dump($v);
}

function baz(int $x = 1, int $y = 2, named int $a = 3, named int $b = 4, named int $c = 5) {
    $v = vec[];
    $v[] = $x;
    $v[] = $y;
    $v[] = $a;
    $v[] = $b;
    $v[] = $c;
    var_dump($v);
}

function qux(int $x, named string $a = "a", named int $b=333, int ...$args) {
    $v = vec[];
    $v[] = $x;
    $v[] = $a;
    $v[] = $b;
    foreach ($args as $arg) {
        $v[] = $arg;
    }
    var_dump($v);
}

<<__EntryPoint>>
function main() {
    foo(1, 2, a=30);
    foo(1, 2, a=30, b=40, c=50);
    foo(1, 2, a=30, c=50);
    foo(1, 2, a=30, b=40);
    foo(1, 2, a=30, b=40, 8, 9, 10, 11);

    bar(1, 2);
    bar(1, 2, a=30);

    baz();
    baz(c=50);

    qux(1);
    qux(1, a="aaa");
    qux(1, b=222);
    qux(1, a="aaa", b=222, 2, 3, 4);

    $v = vec[3, 4];
    qux(1, 2, a="aaa", ...$v);
}

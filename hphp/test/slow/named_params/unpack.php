<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>


function foo(int $x, int $y, named string $a = "a", named int $b=333, int ...$args) {
    $v = vec[];
    $v[] = $x;
    $v[] = $y;
    $v[] = $a;
    $v[] = $b;
    foreach ($args as $arg) {
        $v[] = $arg;
    }
    var_dump($v);
}

function bar(int $x, int $y, named string $a = "a", named int $b=333) {
    $v = vec[];
    $v[] = $x;
    $v[] = $y;
    $v[] = $a;
    $v[] = $b;
    var_dump($v);
}

<<__EntryPoint>>
function main() {
    $v = vec[2, 3, 4, 5, 6, 7];
    foo(...$v);
    foo(1, ...$v);
    foo(1, 2, ...$v);
    foo(...vec[1, 2]);
    foo(...vec[1, 2, 3]);
    foo(...vec[1, 2, 3, 4]);

    bar(...vec[1, 2]);
    bar(1, ...vec[2]);
    bar(1, 2, ...vec[]);
}

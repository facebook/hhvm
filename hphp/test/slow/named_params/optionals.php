<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function foo(named int $a, named int $b, int $opta = 100, int $optb = 200, int ...$args) {
    $v = vec[];
    $v[] = $a;
    $v[] = $b;
    $v[] = $opta;
    $v[] = $optb;
    foreach ($args as $arg) {
        $v[] = $arg;
    }
    var_dump($v);
}

<<__EntryPoint>>
function main() {
  foo(a=3, b=4);
  foo(a=3, 1, b=4);
  foo(a=3, 1, 2, b=4);
  foo(a=3, 1, 2, 3, b=4);
  foo(a=3, 1, 2, 3, 4, b=4);
}

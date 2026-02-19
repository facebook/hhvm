<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function foo<reify T>(named int $a, named int $b, int $opt = 100, int ...$args) {
    $v = vec[];
    $v[] = $a;
    $v[] = $b;
    $v[] = $opt;
    foreach ($args as $arg) {
        $v[] = $arg;
    }
    var_dump($v);
}

<<__EntryPoint>>
function main() {
  foo<int>(a=3, b=4);
  foo<int>(a=3, 1, b=4);
  foo<int>(a=3, 1, 2, b=4);
  foo<int>(a=3, 1, 2, 3, b=4);
}

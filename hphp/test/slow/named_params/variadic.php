<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function foo(named int $a, named int $b, int ...$args) {
    $v = vec[];
    $v[] = $a;
    $v[] = $b;
    foreach ($args as $arg) {
        $v[] = $arg;
    }
    var_dump($v);
}

<<__EntryPoint>>
function main() {
  foo(a=3, 1, 2, b=4);
  foo(a=3, 1, 2, 5, b=4);
  foo(a=3, b=4);

  $v = vec[10, 11, 12];
  foo(a=3, ...$v, b=4);
  foo(...$v, b=4, a=3);
  foo(1, ...$v, b=4, a=3);
  $v = vec[];
  foo(...$v, a=3, b=4);
  foo(1, ...$v, a=3, b=4);
}

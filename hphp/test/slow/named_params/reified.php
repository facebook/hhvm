<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

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

function bar<reify T>(named int $a = 100, named int $b = 200, int ...$args) {
    $v = vec[$a, $b];
    foreach ($args as $arg) {
        $v[] = $arg;
    }
    var_dump($v);
}

function baz<<<__Soft>> reify T>(T $x, named int $a, named int $b = 0) {
    var_dump(vec[$x, $a, $b]);
}

class C {
    public function m<<<__Soft>> reify T>(T $x, named int $a, named int $b = 0) {
        var_dump(vec[$x, $a, $b]);
    }
}

<<__EntryPoint>>
function main() {
  foo<int>(a=3, b=4);
  foo<int>(a=3, 1, b=4);
  foo<int>(a=3, 1, 2, b=4);
  foo<int>(a=3, 1, 2, 3, b=4);

  bar<string>(a=1, b=2);
  bar<string>();
  bar<string>(3);
  bar<string>(3, 4, 5, 6, 7);
  bar<string>(a=1, b=2, 3);

  $v = vec[55, 66];
  bar<string>(a=1, b=2, ...$v);
  bar<string>(a=1);
  bar<string>(b=2, 3);
  bar<string>(b=2, ...$v);

  baz<int>(1, a=2);
  baz<int>(1, a=2, b=3);
  baz<string>(1, a=2);
  baz<string>(1, a=2, b=3);
  baz(1, a=2);
  baz(1, a=2, b=3);

  (new C())->m(1, a=2);
  (new C())->m(1, a=2, b=3);
  (new C())->m<int>(1, a=2, b=3);
  (new C())->m<string>(1, a=2, b=3);
}

<?hh


class Foo {
  public mixed $preInc;
  public mixed $postInc;
  public mixed $preDec;
  public mixed $postDec;
}

function with_exn($fn): void {
  try {
    echo $fn();
  } catch (Exception $e) {
    echo "\n".$e->getMessage()."\n";
  }
}

<<__EntryPoint>>
function main(): void {
  $vals = vec[
    null,
    false,
    true,
    0,
    42,
    1.234,
    'foobar',
    '',
    '1234',
    '1.234',
    HH\stdin(),
  ];
  foreach($vals as $i) {
    $o = new Foo();
    var_dump($i);
    echo "preinc<";
    $o->preInc = __hhvm_intrinsics\launder_value($i);
    with_exn(() ==> ++$o->preInc);
    echo "> postinc<";
    $o->postInc = __hhvm_intrinsics\launder_value($i);
    with_exn(() ==> $o->postInc++);
    echo $o->postInc;
    echo "> predec<";
    $o->preDec = __hhvm_intrinsics\launder_value($i);
    with_exn(() ==> --$o->preDec);
    echo "> postdec<";
    $o->postDec = __hhvm_intrinsics\launder_value($i);
    with_exn(() ==> $o->postDec--);
    echo $o->postDec;
    echo ">\n";
  }
}

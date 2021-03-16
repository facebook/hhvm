<?hh

const vec<mixed> VALS = vec[
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
  STDIN,
];

class Foo {
  public mixed $preInc;
  public mixed $postInc;
  public mixed $preDec;
  public mixed $postDec;
}

<<__EntryPoint>>
function main(): void {
  foreach(VALS as $i) {
    $o = new Foo();
    var_dump($i);
    echo "preinc<";
    $o->preInc = __hhvm_intrinsics\launder_value($i);
    echo ++$o->preInc;
    echo "> postinc<";
    $o->postInc = __hhvm_intrinsics\launder_value($i);
    echo $o->postInc++;
    echo $o->postInc;
    echo "> predec<";
    $o->preDec = __hhvm_intrinsics\launder_value($i);
    echo --$o->preDec;
    echo "> postdec<";
    $o->postDec = __hhvm_intrinsics\launder_value($i);
    echo $o->postDec--;
    echo $o->postDec;
    echo ">\n";
  }
}

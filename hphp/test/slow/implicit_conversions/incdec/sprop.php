<?hh

class Foo {
  public static mixed $preInc;
  public static mixed $postInc;
  public static mixed $preDec;
  public static mixed $postDec;
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
    var_dump($i);
    echo "preinc<";
    Foo::$preInc = $i;
    with_exn(() ==> ++Foo::$preInc);
    echo "> postinc<";
    Foo::$postInc = $i;
    with_exn(() ==> Foo::$postInc++);
    echo Foo::$postInc;
    echo "> predec<";
    Foo::$preDec = $i;
    with_exn(() ==> --Foo::$preDec);
    echo "> postdec<";
    Foo::$postDec = $i;
    with_exn(() ==> Foo::$postDec--);
    echo Foo::$postDec;
    echo ">\n";
  }
}

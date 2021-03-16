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
  public static mixed $preInc;
  public static mixed $postInc;
  public static mixed $preDec;
  public static mixed $postDec;
}

<<__EntryPoint>>
function main(): void {
  foreach(VALS as $i) {
    var_dump($i);
    echo "preinc<";
    Foo::$preInc = $i;
    echo ++Foo::$preInc;
    echo "> postinc<";
    Foo::$postInc = $i;
    echo Foo::$postInc++;
    echo Foo::$postInc;
    echo "> predec<";
    Foo::$preDec = $i;
    echo --Foo::$preDec;
    echo "> postdec<";
    Foo::$postDec = $i;
    echo Foo::$postDec--;
    echo Foo::$postDec;
    echo ">\n";
  }
}

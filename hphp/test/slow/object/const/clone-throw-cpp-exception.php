<?hh

<<__Const>>
class C {
  public static ?C $c = null;

  public function __construct(public int $i)[] {}
  public function __clone(): void {
    $this->i++;

    // stash this and throw a C++ exception
    self::$c = $this;
    exit(0); // implemented by throwing a C++ exception
  }
}

<<__EntryPoint>>
function test() :mixed{
  $c = new C(1);
  var_dump($c);

  register_postsend_function(() ==> {
    $d = C::$c;
    var_dump($d);
    try {
      $d->i++;
      echo "FAIL: wrote to const property on cloned object\n";
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }
    var_dump($d);
  });

  $d = clone $c;
  echo "FAIL: unexpectedly returned from C's clone\n";
}

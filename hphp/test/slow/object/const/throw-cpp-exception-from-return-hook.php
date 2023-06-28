<?hh

<<__Const>>
class C {
  public int $i = 0;
  public vec $v = vec[1];

  public static ?C $c = null;

  public function __construct() {
    echo "-- at constructor entry --\n";
    var_dump($this);
    $this->i = 2;
    $this->v[] = 3;

    // stash this; return hook will throw a C++ exception
    self::$c = $this;
  }
}

<<__EntryPoint>>
function test() :mixed{
  fb_setprofile(($case, $fn) ==> {
    if ($case === 'exit' && $fn === 'C::__construct') {
      exit(0);
    }
  });

  register_postsend_function(() ==> {
    echo "-- after constructor completes --\n";
    $c = C::$c;
    var_dump($c);

    try {
      $c->i = 99;
      echo "FAIL: wrote to scalar property\n";
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }

    try {
      $c->v[] = 999;
      echo "FAIL: wrote through hack array property\n";
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }

    try {
      $c->lol = 'whut';
      echo "FAIL: wrote to dynamic property\n";
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }

    echo "-- at the end --\n";
    var_dump($c);
  });

  new C();
  echo "FAIL: unexpectedly returned from C's constructor\n";
}

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

    // stash this; return hook will throw
    self::$c = $this;
  }
}

<<__EntryPoint>>
function test() :mixed{
  fb_setprofile(($case, $fn) ==> {
    if ($case === 'exit' && $fn === 'C::__construct') {
      throw new Exception('sneaky');
    }
  });

  try {
    $c = new C();
  } catch (Exception $_) {}
  $c = C::$c;
  echo "-- after constructor completes --\n";
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
}

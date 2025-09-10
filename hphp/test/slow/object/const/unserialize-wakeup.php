<?hh

class C {
  public function __construct(<<__Const>> public int $i)[] {}
  public function __wakeup(): void {
    echo "-- in C::__wakeup --\n";
    $this->i++;
  }
}

class D {
  public function __construct(<<__Const>> public int $i, <<__Const>> public C $c)[] {}
  public function __wakeup(): void {
    echo "-- in D::__wakeup --\n";
    $this->i++;
    try {
      $this->c->i = 99;
    } catch (Exception $e) {
      echo $e->getMessage() . "\n";
    }
  }
}

<<__EntryPoint>>
function test() :mixed{
  $d = new D(1, new C(2));
  var_dump($d);
  $d2 = unserialize(serialize($d));
  echo "-- after unserialize --\n";
  var_dump($d2);
  try {
    $d2->i++;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  $c = $d2->c;
  try {
    $c->i = 999;
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
  var_dump($d2);
}

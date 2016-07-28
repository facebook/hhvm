<?hh

const N = 10;

class Obj {
  function __destruct() {
    echo "object destroyed\n";
  }
}

class X {
  public array $arr;
  public int $id;

  public function f() {
    // echo "";
    for (;;) {
      echo "iterating...\n";
      if ($this->id >= N) {
        return;
      }
      $res = $this->arr[$this->id];
      unset($this->arr[$this->id]);
      $this->id++;
    }
  }

  function __construct() {
    for ($i = 0; $i < N; $i++) {
      $this->arr[] = new Obj();
    }
    $this->id = 0;
  }
}

$x = new X();
$x->f();
echo "done\n";

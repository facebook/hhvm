<?hh /* destructor */

const N = 10;

class Obj {
}

class X {
  public AnyArray $arr;
  public int $id;

  public function f() :mixed{
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
    $this->arr = dict[];
    for ($i = 0; $i < N; $i++) {
      $this->arr[] = new Obj();
    }
    $this->id = 0;
  }
}


function f() :mixed{
  $x = new X();
  $x->f();
}

<<__EntryPoint>>
function main_uninit_leak() :mixed{
  f();
  var_dump(HH\objprof_get_data());
  echo "done\n";
}

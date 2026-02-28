<?hh

class C {
  private int $x = 2;
  public function get() :mixed{
    $__lval_tmp_0 = $this->x;
    $this->x++;
    return $__lval_tmp_0;
  }
}

function g() :AsyncGenerator<mixed,mixed,void>{
  $c = new C();
  do {
    $x = $c->get();
    yield $x;
  } while ($x < 5);
}

<<__EntryPoint>>
function test() :mixed{
  $g1 = g();
  foreach ($g1 as $x) {
    var_dump($x);
    break;
  }

  $g2 = clone($g1);
  unset($g1);
  gc_collect_cycles();

  foreach($g2 as $x) {
    var_dump($x);
  }
}

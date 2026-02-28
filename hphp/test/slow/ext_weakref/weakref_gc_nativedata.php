<?hh
// Source php weakref extension
class Foo {
  private $ptr = null;
  private $dt = null;
  public function set($p) :mixed{
    $this->ptr = $p;
  }
  public function getNativeDataWeakRef() :mixed{
    $dt = new DateTime();
    $this->dt = $dt;
    return new WeakRef($dt);
  }
}
function foo() :mixed{
  $a = new Foo();
  $b = new Foo();
  $a->set($b);
  $b->set($a);
  return $a->getNativeDataWeakRef();
}

class Leaker {
  private $ptr = null;
  private $dt = null;
  public function set($p) :mixed{
    $this->ptr = $p;
  }
  public function getNativeDataWeakRef() :mixed{
    $dt = new DateTime();
    $this->dt = $dt;
    return new WeakRef($dt);
  }
};

function leak() :mixed{
  $l1 = new Leaker();
  $l2 = new Leaker();
  $l1->set($l2);
  $l2->set($l1);
  return $l1->getNativeDataWeakRef();
}

function factorial($n) :mixed{
  if ($n <= 1) return 1;
  return $n * factorial($n-1);
}

<<__EntryPoint>>
function main() :mixed{
  $wr = leak();

  var_dump($wr->valid(), $wr->get());

  // We need to cleanup tvBuiltinReturn, we do this by creating a garbage object,
  // and just passing it around. (Otherwise, it's possible one of the Leakers
  // above could be in tvBuiltinReturn, and marked as reachable.)
  $_ = foo();  // clear tvReturn.
  // Similarly, we need to make sure the PHP stack doesn't have an object that's
  // accidentally reachable if the stack is conservatively scanned.
  factorial(10);  // clears the stack.

  gc_collect_cycles();

  var_dump($wr->valid());
  var_dump($wr->get());
}

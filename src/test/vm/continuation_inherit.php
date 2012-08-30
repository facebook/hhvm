<?

class Sub extends GenericContinuation {
  public $a;
  public $b;
  public $c;
  public $d;
  public $foo;
  public $barg;
  public $snarg;
}

class HasDtor {
  public function __destruct() {
    echo "hootenanny.\n";
  }
}


$sub = new Sub;
$sub->a = 0xdeadbeef;
$sub->b = 0xdeadbeef;
$sub->c = new HasDtor();
$sub->d = 0xdeadbeef;
$sub->foo = 0xdeadbeef;
$sub->barg = 0xdeadbeef;
$sub->snarg = 0xdeadbeef;
$sub = null;

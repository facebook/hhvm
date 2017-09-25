<?hh

class ParentClass {
  private $arr = array('hello');
  private $str = 'blep';
  protected $prot = 'bloop';
}

class WithStuff extends ParentClass {
  private $arr = array(1);
  private $vec = Vector {'str', 12.3, 123};
  public $pub = 'lollerskates';
  protected $prot = 'blorp';

  <<__Memoize>>
  function add($a, $b) {
    echo "Adding $a and $b\n";
    return $a + $b;
  }
}

if (apc_exists('foo')) {
  class X {
    private $foo;
    function __construct() { $this->foo='bar'; }
    function show() { var_dump($this->foo); }
  };

  var_dump(apc_fetch('foo'));
} else {
  class X {
    private $foo;
    function __construct() { $this->foo='bar'; }
    function show() { var_dump($this->foo); }
  }
  apc_store('foo', new X);
  var_dump(apc_fetch('foo'));

  $o = new WithStuff();
  $o->dynProp = $o->add(1, 2);
  $o->add(1, 2);

  apc_store('o', $o);
  var_dump($o);
  $o2 = apc_fetch('o');
  var_dump($o2);
  $o2->add(1, 2);
  $o2->add(1, 2);

  apc_fetch('o');
  $o2 = apc_fetch('o');
  var_dump($o2);
  $o2->add(1, 2);
  $o2->add(1, 2);
}

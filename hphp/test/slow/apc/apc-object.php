<?hh

class ParentClass {
  private $arr = vec['hello'];
  private $str = 'blep';
  protected $prot = 'bloop';
}

class WithStuff extends ParentClass {
  private $arr = vec[1];
  private $vec = Vector {'str', 12.3, 123};
  public $pub = 'lollerskates';
  protected $prot = 'blorp';

  <<__Memoize>>
  function add($a, $b) :mixed{
    echo "Adding $a and $b\n";
    return $a + $b;
  }
}
<<__EntryPoint>>
function entrypoint_apcobject(): void {

  if (apc_exists('foo')) {
    include 'apc-object-1.inc';
    var_dump(__hhvm_intrinsics\apc_fetch_no_check('foo'));
  } else {
    include 'apc-object-2.inc';
    apc_store('foo', new X);
    var_dump(__hhvm_intrinsics\apc_fetch_no_check('foo'));

    $o = new WithStuff();
    $o->dynProp = $o->add(1, 2);
    $o->add(1, 2);

    apc_store('o', $o);
    var_dump($o);
    $o2 = __hhvm_intrinsics\apc_fetch_no_check('o');
    var_dump($o2);
    $o2->add(1, 2);
    $o2->add(1, 2);

    __hhvm_intrinsics\apc_fetch_no_check('o');
    $o2 = __hhvm_intrinsics\apc_fetch_no_check('o');
    var_dump($o2);
    $o2->add(1, 2);
    $o2->add(1, 2);
  }
}

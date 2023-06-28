<?hh

class C0 {
  public $a;
  function __construct($a) {
    $this->a = $a;
  }
}

function test_clsname(classname<C0> $c): void {
  var_dump($c);
}

function test_basic_new_clsref(classname<C0> $c, $v): void {
  $x = new $c($v);
  var_dump($x);
}

function test_apc_str($key, $clsname, $v, $isDynamic) :mixed{
  apc_store($key, $clsname);
  if ($isDynamic) {
    eval(
      "class ".$clsname.
      " { public \$a; function __construct(\$a) { \$this->a = \$a; } }");
  }
  test_basic_new_clsref(__hhvm_intrinsics\apc_fetch_no_check($key), $v);
  apc_delete($key);
}

<<__EntryPoint>>
function main() :mixed{
  test_clsname(C0::class);
  test_clsname('C0');
  test_basic_new_clsref(C0::class, 1);
  test_basic_new_clsref('C0', 2);
  test_basic_new_clsref(__hhvm_intrinsics\launder_value('C0'), 3);

  $x = 'C';
  test_basic_new_clsref($x.__hhvm_intrinsics\launder_value('0'), 4);

  // APC store will treat this C0 string as static.
  test_apc_str('k0',  $x.__hhvm_intrinsics\launder_value('0'), 5, false);

  // To make sure apc will make shared string (uncounted string),
  // class needed to be created dynamically.
  test_apc_str('k0',  $x.__hhvm_intrinsics\launder_value('1'), 5, true);
}

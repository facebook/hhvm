<?hh

class C {
  public static function f(): void { echo " called\n"; }
}

function check_array($key, $arr): void {
  apc_store($key, $arr);
  $a = __hhvm_intrinsics\apc_fetch_no_check($key);
  foreach ($a as $k => $v) {
    echo "$k:";
    $v::f();
  }
}

<<__EntryPoint>>
function main(): void {
  $ss = nameof C;
  $s = __hhvm_intrinsics\launder_value($ss)."";
  $lc = C::class;
  $c = HH\classname_to_class($lc);

  $collection = Map {
    'static_string' => $ss,
    'string' => $s,
    'lazy_class' => $lc,
    'class' => $c,
  };

  echo "===== Basic =====\n";
  foreach ($collection as $k => $v) {
    apc_store($k, $v);
    $x = __hhvm_intrinsics\apc_fetch_no_check($k);
    echo "$k:";
    $x::f();
  }

  echo "\n\n===== Map =====\n";
  check_array('collection', $collection);

  echo "\n\n===== dict =====\n";
  $dict = dict($collection);
  check_array('dict', $dict);

  echo "\n\n===== Pair =====\n";
  foreach ($collection as $k => $v) {
    $pair = Pair { $k, $v };
    apc_store('pair', $pair);
    $p = __hhvm_intrinsics\apc_fetch_no_check('pair');
    $k = $p[0];
    $v = $p[1];
    echo "$k:";
    $v::f();
  }
}

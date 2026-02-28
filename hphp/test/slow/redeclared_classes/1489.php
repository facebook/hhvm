<?hh

function foo($x) :mixed{
  $s = serialize($x);
  var_dump($s);
  $y = unserialize($s);
  var_dump($y);
  if (function_exists('apc_store')) {
    apc_store('foo', $y);
    $z = __hhvm_intrinsics\apc_fetch_no_check('foo');
  }
  else {
    $z = clone $y;
  }
  var_dump($z);
  unset($z, $y);
  var_dump($x);
}
<<__EntryPoint>>
function entrypoint_1489(): void {

  if (isset($g)) {
    include '1489-1.inc';
  }
  else {
    include '1489-2.inc';
  }

  include '1489-classes.inc';

  $y = new y;
  foo($y);
  $z = new z;
  foo($z);
  unset($z, $y);
}

<?hh

function foo() {
  $count = __hhvm_intrinsics\apc_fetch_no_check('rec-parent-count');
  if ($count === false) $count = 0;
  if ($count >= 2) return;

  if ($count == 0) {
    require 'record-parent-1.inc';
  } else {
    require 'record-parent-2.inc';
  }
  require 'record-child.inc';

  $a = D['y' => 42];
  var_dump($a['x']);
  var_dump($a['y']);

  $count++;
  apc_store('rec-parent-count', $count);
}

<<__EntryPoint>>
function main() {
  foo();
}

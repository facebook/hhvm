<?hh

<<__EntryPoint>>
function main() :mixed{
  $count = (int)__hhvm_intrinsics\apc_fetch_no_check('count') + 1;
  if ($count > 8) return;
  apc_store('count', $count);
  echo "====================== $count =======================\n";
  require_once(__DIR__.'/disable_superglobals'.$count.'.php.inc');
}

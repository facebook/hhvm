<?hh


<<__EntryPoint>>
function parse_errors_main() :mixed{
  $count = (int)__hhvm_intrinsics\apc_fetch_no_check('count') + 1;
  if ($count > 7) return;
  apc_store('count', $count);
  echo "====================== $count =======================\n";
  require_once(__DIR__.'/parse_errors'.$count.'.php.inc');
}

<?hh

<<__EntryPoint>>
function main() :mixed{
  $count = __hhvm_intrinsics\apc_fetch_no_check('count');
  if ($count === false) $count = 0;
  if ($count < 4) {
    ++$count;
    apc_store('count', $count);
    echo "====================== $count =======================\n";
    require_once(__DIR__.'/func-ptr-in-const-disallowed'.$count.'.php.inc');
  }
}

<?hh

<<__EntryPoint>>
function main_11() {
  if (apc_size('key_does_not_exist') !== null) echo "no\n";
  apc_store('key_exists', darray['000' => varray['1','2','3','4','5']]);

  if (apc_size('key_exists') <= 0) echo "no\n";
  echo "ok\n";
}

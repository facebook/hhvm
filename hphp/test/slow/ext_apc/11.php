<?hh

<<__EntryPoint>>
function main_11() :mixed{
  if (apc_size('key_does_not_exist') !== null) echo "no\n";
  apc_store('key_exists', dict['000' => vec['1','2','3','4','5']]);

  if (apc_size('key_exists') <= 0) echo "no\n";
  echo "ok\n";
}

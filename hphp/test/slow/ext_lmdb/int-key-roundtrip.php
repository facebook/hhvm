<?hh
<<__EntryPoint>>
function main(): void {
  // An int-key put/get must round-trip.
  $dir = sys_get_temp_dir() . '/hhvm_lmdb_intkey_' . (string)getmypid();
  if (!is_dir($dir)) { mkdir($dir); }
  $env = \HH\lmdb\mdb_env_create();
  \HH\lmdb\mdb_env_open($env, $dir, 0, 0644);
  $txn = \HH\lmdb\mdb_txn_begin($env, null, 0);
  $dbi = \HH\lmdb\mdb_dbi_open($txn, null, \HH\lmdb\MDB_DBI_FLAGS::MDB_CREATE);
  $data = "hello-int-key";
  \HH\lmdb\mdb_put($txn, $dbi, 12345, inout $data, 0);
  $out = null;
  \HH\lmdb\mdb_get($txn, $dbi, 12345, inout $out);
  var_dump($out);
}

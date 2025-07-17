<?hh

<<__EntryPoint>>
async function main() {
  $used_bytes = rds_bytes();

  var_dump($used_bytes['used_bytes'] > 0);
  var_dump($used_bytes['used_local_bytes'] > 0);
  var_dump($used_bytes['used_persistent_bytes'] > 0);
}


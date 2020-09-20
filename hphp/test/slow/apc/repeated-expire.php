<?hh

<<__EntryPoint>>
function main(): void {
  $ok = false;

  for ($i = 0; $i < 5; $i++) {
    apc_add('cur', $i, 1);
    sleep(2);
    $cur = apc_fetch('cur', inout $ok);
    if ($cur !== false) {
      echo "Did not expire round $i value $cur\n";
      return;
    }
  }
  echo "OK\n";
}

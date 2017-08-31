<?hh

function makeNonStatic($n) {
  $s = 'foobar';
  for ($i = 0; $i < $n; $i += 1) {
    $s .= $i;
  }
  return $s;
}

function main() {
  $a = array();
  // Insert an object to force $a to become ref-counted in APC.
  $a['obj'] = new stdClass;
  $a['unc'] = makeNonStatic(10);
  var_dump($a);
  apc_store('counted', $a);
  $unc = apc_fetch('counted')['unc'];
  // This eagerly deletes the outer array...
  apc_delete('counted');
  // ...but the uncounted string inside must stay until end of this request.
  var_dump($unc);
}

main();

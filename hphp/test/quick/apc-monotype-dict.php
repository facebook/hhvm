<?hh

function makeNonStatic($n) :mixed{
  $s = 'foobar';
  for ($i = 0; $i < $n; $i += 1) {
    $s .= $i;
  }
  return $s;
}

<<__EntryPoint>> function main(): void {
  $a = dict[];
  // Insert an object to force $a to become ref-counted in APC.
  $a[makeNonStatic(17)] = makeNonStatic(34);
  $a[makeNonStatic(51)] = makeNonStatic(68);
  var_dump($a);
  apc_store('counted', $a);
  $unc = vec(__hhvm_intrinsics\apc_fetch_no_check('counted'))[1];
  // This eagerly deletes the outer array...
  apc_delete('counted');
  // ...but the uncounted string inside must stay until end of this request.
  var_dump($unc);
}

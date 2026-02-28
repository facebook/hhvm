<?hh

<<__EntryPoint>>
function main(): void {
  $d = dict[];
  $d[17] = Vector { 'foo', 'bar' };
  $d[34] = 51;
  apc_add('d', $d);

  $v = dict[];
  $v[] = Vector { 17, 34 };
  $v[] = 68;
  apc_add('v', $v);

  // Mutate the old values, which shouldn't affect the APC copies.
  $d[17]->append('baz');
  $v[0]->append(51);

  // Verify that we can retrieve the stored values.
  $success = null;
  $d_fetch = apc_fetch('d', inout $success);
  $v_fetch = apc_fetch('v', inout $success);
  var_dump($d_fetch);
  var_dump($v_fetch);
}

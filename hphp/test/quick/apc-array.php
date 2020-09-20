<?hh

<<__EntryPoint>>
function main(): void {
  // A "plain" array is a PHP array that is not a darray or varray.
  //
  // Create a plain mixed-layout array and store it in APC.
  $d = __hhvm_intrinsics\dummy_cast_to_kindofarray(darray[]);
  $d[17] = Vector { 'foo', 'bar' };
  $d[34] = 51;
  apc_add('d', $d);

  // Create a plain packed-layout array and store it in APC.
  $v = __hhvm_intrinsics\dummy_cast_to_kindofarray(varray[]);
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

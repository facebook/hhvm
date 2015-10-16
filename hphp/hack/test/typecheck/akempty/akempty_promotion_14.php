<?hh //strict

/**
 * Could be no-error, but the typechecker doesn't track copy-by-value semantics
 * of array when dealing with nested types =(
 */
function test(): ConstVector<array<array<string, string>>> {
  $v = Vector {array(array())};
  $a = $v[0];
  $a[0]['aaa'] = 4;
  return $v;
}

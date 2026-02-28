<?hh

/**
 * Test AKempty promotion in double-nested collection with intermediate
 * assignment
 */

function test(): ConstVector<ConstVector<darray<string, string>>> {
  $v1 = Vector { Vector { dict[] } };
  $v2 = $v1[0];
  $v3 = $v2[0];
  $v3['aaa'] = 4;
  // no error - "$v3 = " assignment copies the array,
  // and final modification doesn't affect $v1
  return $v1;
}

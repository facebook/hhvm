<?hh

/**
 * Test AKempty promotion in double-nested collection
 */
function test(): ConstVector<ConstVector<darray<string, string>>> {
  $v1 = Vector { Vector { dict[] } };
  $v2 = $v1[0];
  $v2[0]['aaa'] = 4;
  // Vectors have reference semantics, so should be an error - $v1 will
  // contain integers now
  return $v1;
}

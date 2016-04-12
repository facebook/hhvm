<?hh //strict

/**
 * Test AKempty promotion affecting an array inside a collection
 */
function test(): ConstVector<array<string, string>> {
  $v = Vector {array()};
  $v[0]['aaa'] = 4;
  // should be an error - $v can contain strings integers now
  return $v;
}

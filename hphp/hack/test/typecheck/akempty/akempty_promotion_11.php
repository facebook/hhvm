<?hh //strict

/**
 * Test AKempty promotion affecting an array inside a collection with
 * intermediate assignment
 */
function test(): ConstVector<array<string, string>> {
  $v = Vector {array()};
  $a = $v[0];
  $a['aaa'] = 4;
  // no error "$a =" assignment created a copy of an array inside $v, and
  // further modifications don't affect it
  return $v;
}

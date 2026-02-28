<?hh
class :foo extends XHPTest {
  // Typechecker must consider this nullable
  attribute int x;
}
function test1(:foo $obj): int {
  return $obj->:x;
}

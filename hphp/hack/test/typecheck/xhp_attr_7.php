<?hh // strict
class :foo extends XHPTest {
  // Typechecker must consider this nullable
  attribute int x = null;
}
function test1(:foo $obj): int {
  // Typechecker should detect a mismatch here
  return $obj->:x;
}

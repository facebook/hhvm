<?hh // strict
class :foo extends XHPTest {
  attribute int x = 0;
  public string $x = "";
}
function test1(:foo $obj): int {
  return $obj->:x;
}
function test2(:foo $obj): string {
  return $obj->x;
}

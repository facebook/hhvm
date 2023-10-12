<?hh // strict
class :baz {
  attribute int x @required;
}
class :bar {
  // Import all attributes from :baz
  attribute :baz;
}
class :foo extends XHPTest {
  // Import all attributes from :bar
  attribute :bar;
}
function test1(:foo $obj): string {
  // Test if the typechecker understands attribute importation
  // and can detect a type mismatch here
  return $obj->:x;
}

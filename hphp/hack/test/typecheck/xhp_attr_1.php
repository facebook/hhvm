<?hh // strict
class :blah {}
class GenericClass<T> {}
class SomeClass {
  public function bar(): float { return 3.14; }
}
class :foo {
  attribute
    // Using array without type parameters is okay for now
    array bar-baz = array(),
    // Using GenericClass without type parameters is okay for now
    GenericClass yo @required,
    // Check to make sure that "->:" parses correctly when chained
    // together in a larger expression.
    SomeClass some-obj @required,
    // An XHP class name can be used as a type hint; marking it
    // with "@required" so the typechecker knows it can't be null
    :blah fizz-buzz @required,
    // If it's not marked "@required" and is doesn't have a non-null
    // default value, then the typechecker treats it as nullable
    :blah might:be-null,
    // Make sure the weird trait importation syntax works when
    // interleaved with normal XHP attribute declarations
    :blah,
    // An XHP attribute annoted as "string" can contain Stringish
    // values for now
    string string:attr-might-be:stringish = '';
}
function test1(:foo $obj): :blah {
  return $obj->:fizz-buzz;
}
function test2(:foo $obj): ?:blah {
  return $obj->:name:with:col;
}
function test3(:foo $obj): Stringish {
  return $obj->:string:attr-might-be:stringish;
}
function test4(:foo $obj): float {
  return $obj->:some-obj->bar();
}

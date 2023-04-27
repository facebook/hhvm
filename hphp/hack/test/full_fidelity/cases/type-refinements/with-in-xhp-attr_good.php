<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

class :simple-type extends XHPTest {
  attribute bool with-b;
}
function simple_attr(:simple-type $obj): bool {
  return $obj->:with-b;
}

class :nonsimple-types extends XHPTest {
  attribute
    ?bool with-b1,
    ?bool with-b2;
}
function nonsimple_attrs(:nonsimple-types $obj): bool {
  return $foo->:with-b1 + $foo->:with-b2;
}

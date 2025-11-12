<?hh
<<file:__EnableUnstableFeatures('xhp_type_constants')>>

class A {
  const type T = int;
}

class :x extends XHPTest {
  attribute A::T y @required;
}

function test(:x $obj): int {
  return $obj->:y;
}

function create_instance(): :x {
  return <x y={42} />;
}

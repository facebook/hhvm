<?hh

function test(
  Vector $v_empty,
  Map $m_not_empty,
  mixed $unknown_empty,
  mixed $unknown_not_empty
): void {
  var_dump((bool)$v_empty);
  var_dump((bool)$m_not_empty);
  var_dump((bool)$unknown_empty);
  var_dump((bool)$unknown_not_empty);

  var_dump((bool)(Vector{}));
  var_dump((bool)(Vector{1}));

  var_dump((bool)(Map{}));
  var_dump((bool)(__hhvm_intrinsics\launder_value(Map{})));
}

<<__EntryPoint>>
function main() :mixed{
  for ($i = 0; $i < 3; $i++) {
    test(
      Vector{},
      Map{0 => 1},
      __hhvm_intrinsics\launder_value(Set{}),
      __hhvm_intrinsics\launder_value(Vector{1})
    );
  }
}

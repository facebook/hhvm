<?hh

<<__EntryPoint>>
function main(): void {
  $values = vec[
    vec[],
    vec[],
    42,
    Vector {},
  ] |> __hhvm_intrinsics\launder_value($$);
  $fn = __hhvm_intrinsics\launder_value(is_array<>);

  foreach ($values as $v) {
    echo " --- Direct is_array call --- \n";
    $_ = is_array($v);
    echo " --- Indirect is_array call --- \n";
    $_ = $fn($v);
  }
}

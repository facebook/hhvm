<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

function test(mixed $value): void {
  // `nothing` absorbs the result, but validation must still record the invalid
  // `mixed` operand.
  var_dump($value is shape(...nothing, ...?dynamic));
}

<<__EntryPoint>>
function main(): void {
  test(__hhvm_intrinsics\launder_value(null));
}

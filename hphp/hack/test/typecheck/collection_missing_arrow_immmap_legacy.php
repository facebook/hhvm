<?hh

function test(): void {
  // Missing arrow in a key-value collection (ImmMap)
  $x = ImmMap {'just_value'};
}

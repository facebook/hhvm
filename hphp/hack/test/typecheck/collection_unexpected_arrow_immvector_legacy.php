<?hh

function test(): void {
  // Using key => value in a value-only collection (ImmVector)
  $x = ImmVector {'key' => 'value'};
}

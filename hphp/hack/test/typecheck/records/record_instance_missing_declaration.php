<?hh


function foo(): void {
  $bar = NoSuchRecord['x' => 1];
  $baz = NoSuchRecord@['x' => 1];
}

<?hh

function test(HH\EnumClass\Label<nothing, mixed> $label): void {
  if ($label is string) {
    hh_expect<nothing>($label);
  }
}

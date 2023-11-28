<?hh

function main(): void {
  $a = HH\FIXME\UNSAFE_CAST<int, string>(42);
  hh_expect<bool>($a);
}

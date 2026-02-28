<?hh

function test_set(ConstSet<string> $set, string $key): void {
  $_ = $set[$key];
  //       ^ at-caret
}

<?hh //strict

function foos(): array<string> {
  return array();
}
function bars(): array<int> {
  return array();
}

function test_a(): array<num> {
  $a = foos();
  $a += bars();
  return $a;
}

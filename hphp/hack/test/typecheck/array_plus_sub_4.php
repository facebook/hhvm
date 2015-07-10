<?hh //strict

function foos(): array<string> {
  return array();
}
function bars2(): array<string, int> {
  return array();
}

function test_a(): array<mixed> {
  $a = foos();
  $a += bars2();
  return $a;
}

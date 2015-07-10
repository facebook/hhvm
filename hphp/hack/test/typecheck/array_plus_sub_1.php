<?hh //strict

function foos(): array<string> {
  return array();
}
function bars(): array<int> {
  return array();
}
function foos2(): array<string, string> {
  return array();
}
function bars2(): array<string, int> {
  return array();
}

function test_a(): array<mixed> {
  $a = foos();
  $a += bars();
  return $a;
}
function test_a2(): array<mixed, mixed> {
  $a = foos2();
  $a += bars2();
  return $a;
}

function test_b(): array<mixed> {
  return foos() + bars();
}
function test_b2(): array<string, mixed> {
  return foos2() + bars2();
}

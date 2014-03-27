<?hh // strict

namespace NS1\NS2 {
  const int X = 0;
}

function test(): int {
  return \NS1\NS2\X;
}

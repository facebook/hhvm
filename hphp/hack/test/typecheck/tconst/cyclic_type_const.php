<?hh // strict

class C1 {
  const type T = C2::T;
}

class C2 {
  const type T = C1::T;
}

type Cycle = C1::T;

function test(): Cycle {
}

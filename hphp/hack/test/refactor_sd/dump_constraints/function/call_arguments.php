<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

function f(): int {
  return 1;
}

function g(int $i): void {}

function h(): void {
  g((f<>)());
}

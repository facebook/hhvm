<?hh

function good(): void {
  // There shouldn't be an error
  (int $_, string $_) ==> {};
}

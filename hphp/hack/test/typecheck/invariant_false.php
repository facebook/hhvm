<?hh // strict

function test(): int {
  invariant(false, 'This should be an invariant violation');
}

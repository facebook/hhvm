<?hh // strict

function n(): noreturn {
  throw new Exception('nope');
}

function test(): void {
  if (n()) {
  }
}

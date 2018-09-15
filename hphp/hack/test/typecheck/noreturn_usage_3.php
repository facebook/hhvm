<?hh // strict

function n(): noreturn {
  throw new Exception('nope');
}

function test(): void {
  while (n()) {
  }
}

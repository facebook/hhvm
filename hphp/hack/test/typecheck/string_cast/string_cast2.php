<?hh // strict

class CNonStringish {}

function test(): void {
  $cnon = new CNonStringish();

  (string) $cnon;
}

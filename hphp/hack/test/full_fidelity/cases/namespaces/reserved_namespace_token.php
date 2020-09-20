<?hh

// Using `namespace` in any case as a name is not allowed

class Namespace {}

function test(): void {
  $y = new Namespace();
}

function Namespace(): void {}

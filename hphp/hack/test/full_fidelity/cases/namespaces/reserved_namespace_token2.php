<?hh

// Using `namespace` as a name is not allowed

class namespace {}

function test(): void {
  $x = new namespace();
}

function namespace(): void {}

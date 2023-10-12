<?hh // strict

function herp(inout int $x): void {
  $x += 42;
}

function derp(): void {
  $z = 'derp';
  herp(inout $z);
}

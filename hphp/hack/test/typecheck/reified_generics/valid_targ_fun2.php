<?hh

function f<reify T>(T $x): void {}

function main(): void {
  f<(function(mixed):mixed)>(1);
}

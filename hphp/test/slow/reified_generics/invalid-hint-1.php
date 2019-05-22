<?hh

function g<reify T>(): void {
  new D<(function (): string)>() is T;
}

class D<reify T> {}

<<__EntryPoint>>
function main(): void {
  g<D<(function (): int)>>();
}

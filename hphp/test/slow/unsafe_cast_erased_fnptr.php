<?hh

function f(mixed $m): void {}

<<__EntryPoint>>
function main(): void {
  f(unsafe_cast<int, string>);
}

<?hh

function g<T>(): void { echo "done\n"; }

function f<T>(): void {
  g<(function (T...): mixed)>();
}

<<__EntryPoint>>
function main(): void {
  f<mixed>();
}

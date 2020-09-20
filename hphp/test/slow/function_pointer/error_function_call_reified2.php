<?hh

function f<reify T>() {}

<<__EntryPoint>>
function main(): void {
  $f = f<int, string>;
}

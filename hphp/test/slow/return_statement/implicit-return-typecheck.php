<?hh
function foo(): int {}
function main() {
  foo();
}

<<__EntryPoint>>
function main_implicit_return_typecheck() {
main();
}

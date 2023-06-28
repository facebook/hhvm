<?hh
function foo(): int {}
function main() :mixed{
  foo();
}

<<__EntryPoint>>
function main_implicit_return_typecheck() :mixed{
main();
}

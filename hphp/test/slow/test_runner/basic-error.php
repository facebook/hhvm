<?hh

function foo(): int {
  return 'Hi';
}


<<__EntryPoint>>
function main_basic_error() :mixed{
foo();
}

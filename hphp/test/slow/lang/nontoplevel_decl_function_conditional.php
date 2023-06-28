<?hh

<<__EntryPoint>>
function main() :mixed{
  if (true) {
    function f(): void {} // bad
  } else {
    function g(): void {} // bad
  }
}

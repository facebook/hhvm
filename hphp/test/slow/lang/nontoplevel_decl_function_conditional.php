<?hh

<<__EntryPoint>>
function main() {
  if (true) {
    function f(): void {} // bad
  } else {
    function g(): void {} // bad
  }
}

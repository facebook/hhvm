<?hh

<<__EntryPoint>>
function main() {
  if (true) {
    class A {} // bad
  } else {
    class B {} // bad
  }
}

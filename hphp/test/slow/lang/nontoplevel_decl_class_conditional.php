<?hh

<<__EntryPoint>>
function main() :mixed{
  if (true) {
    class A {} // bad
  } else {
    class B {} // bad
  }
}

<?hh

class C {}

function reified_and_variadic<reify T>(...$v) {}

<<__EntryPoint>>
function main() {
  reified_and_variadic<bool>(new C(), new C());
}

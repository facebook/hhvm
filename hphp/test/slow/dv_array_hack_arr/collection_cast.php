<?hh

class C {}

<<__EntryPoint>>
function main() {
  var_dump(array_reverse(Map { 'a' => new C() }));
}

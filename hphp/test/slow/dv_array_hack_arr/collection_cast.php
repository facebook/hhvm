<?hh

class C {}

<<__EntryPoint>>
function main() :mixed{
  var_dump(array_reverse(Map { 'a' => new C() }));
}

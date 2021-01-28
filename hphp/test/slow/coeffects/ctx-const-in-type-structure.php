<?hh

class A {
  const ctx C = [rx];
}

<<__EntryPoint>>
function main() {
  var_dump(type_structure(A::class, 'C'));
}

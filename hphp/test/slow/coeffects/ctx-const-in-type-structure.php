<?hh

class A {
  const ctx C = [rx];
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(type_structure(A::class, 'C'));
}

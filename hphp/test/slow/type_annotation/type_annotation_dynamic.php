<?hh

class C {
  const type A = dynamic;
}

<<__EntryPoint>>
function main() {
  $x = new ReflectionTypeConstant(C::class, 'A');
  var_dump($x->getAssignedTypeText());
  var_dump($x->getTypeStructure());
}

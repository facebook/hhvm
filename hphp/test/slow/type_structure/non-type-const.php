<?hh

class C {
  const int CNS = 10;
}

<<__EntryPoint>> function main() :mixed{
  var_dump(type_structure(C::class, 'CNS'));
}

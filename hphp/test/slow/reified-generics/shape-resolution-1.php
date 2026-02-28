<?hh

class C {
  const foo = "A";
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(HH\ReifiedGenerics\get_type_structure<shape("foo"=> string)>());
  var_dump(HH\ReifiedGenerics\get_type_structure<shape(?"foo"=> string)>());
  var_dump(HH\ReifiedGenerics\get_type_structure<shape(C::foo=> string)>());
}

<?hh


interface I {
  abstract const X;
  const Y = 'I::Y';
  const Z = self::Y . ' via Z';
}


<<__EntryPoint>>
function main_abstract_const5() :mixed{
var_dump(I::X); // no value!
}

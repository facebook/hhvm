<?hh
class :fb:thing implements XHPChild {
  const X = 12;
}

const T = :fb:thing::class;
const U = :fb:thing::X;


<<__EntryPoint>>
function main_class_name() :mixed{
var_dump(U);
var_dump(T);
}

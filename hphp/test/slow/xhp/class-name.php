<?hh
class :fb:thing implements XHPChild {
  const X = 12;
}

const T = :fb:thing::class;
const U = :fb:thing::X;

var_dump(U);
var_dump(T);
